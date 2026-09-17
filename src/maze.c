#include "maze.h"
#include "rlgl.h"
#include <string.h>
#include <math.h>

typedef enum { DIR_N, DIR_S, DIR_E, DIR_W } Dir;

typedef struct {
    Dir dir;
    int steps;
} CarveStep;

static void StepCursor(Dir d, int *cx, int *cz) {
    switch (d) {
        case DIR_N: (*cz)--; break;
        case DIR_S: (*cz)++; break;
        case DIR_E: (*cx)++; break;
        case DIR_W: (*cx)--; break;
    }
}

// Builds a maze from a start cell + a sequence of carve steps.
//   seq[0 .. kioskStepIndex-1]   : an approach corridor, carved as plain
//                                  floor (walkable from the very start, no
//                                  gate) - used by level 1's long hallway
//                                  that leads up to its terminal.
//   seq[kioskStepIndex]          : ends on the kiosk cell.
//   seq[kioskStepIndex+1]        : carved as the locked gate (CELL_BLOCK)
//                                  that opens once the kiosk's minigame is
//                                  solved.
//   seq[kioskStepIndex+2 .. end] : the guided corridor to the exit; every
//                                  carved cell is recorded into the path
//                                  list (used to draw "which way to go"
//                                  arrow decals once the gate opens).
static void BuildMaze(Maze *m, int width, int height, int startCol, int startRow,
                       float startYawDeg, Color wallColor, Color floorColor,
                       const CarveStep *seq, int seqCount, int kioskStepIndex) {
    memset(m, 0, sizeof(*m));
    m->width = width;
    m->height = height;
    m->wallColor = wallColor;
    m->floorColor = floorColor;
    m->startYawDeg = startYawDeg;

    for (int r = 0; r < height; r++) {
        for (int c = 0; c < width; c++) {
            m->cells[r][c] = CELL_WALL;
        }
    }

    int cx = startCol, cz = startRow;
    m->startCol = cx;
    m->startRow = cz;
    m->cells[cz][cx] = CELL_FLOOR;

    // Approach corridor before the kiosk (plain floor, no gate).
    for (int s = 0; s < kioskStepIndex; s++) {
        for (int i = 0; i < seq[s].steps; i++) {
            StepCursor(seq[s].dir, &cx, &cz);
            m->cells[cz][cx] = CELL_FLOOR;
        }
    }

    // Kiosk cell(s) - the final carved cell of this entry is the kiosk.
    for (int i = 0; i < seq[kioskStepIndex].steps; i++) {
        StepCursor(seq[kioskStepIndex].dir, &cx, &cz);
        m->cells[cz][cx] = CELL_FLOOR;
    }
    m->kioskCol = cx;
    m->kioskRow = cz;

    // Locked gate cell(s).
    for (int i = 0; i < seq[kioskStepIndex + 1].steps; i++) {
        StepCursor(seq[kioskStepIndex + 1].dir, &cx, &cz);
        m->cells[cz][cx] = CELL_BLOCK;
    }

    // Remaining steps: the guided corridor to the exit.
    m->pathLen = 0;
    for (int s = kioskStepIndex + 2; s < seqCount; s++) {
        for (int i = 0; i < seq[s].steps; i++) {
            StepCursor(seq[s].dir, &cx, &cz);
            m->cells[cz][cx] = CELL_FLOOR;
            if (m->pathLen < MAZE_MAX_PATH) {
                m->pathCol[m->pathLen] = cx;
                m->pathRow[m->pathLen] = cz;
                m->pathLen++;
            }
        }
    }
    m->exitCol = cx;
    m->exitRow = cz;
}

// Builds a branching maze (levels 2-5): a short foyer with a kiosk and a
// locked gate (same as BuildMaze's foyer), followed by a real maze carved
// with a randomized recursive-backtracker over an 8x8 grid of rooms (64
// rooms), plus a braiding pass that knocks down some extra walls to add
// genuine loops on top of the tree - so a player who wanders instead of
// following the arrows finds real junctions, dead ends, and alternate
// routes, not just one hallway. The backtracker's spanning tree always
// reaches every room (including the exit), so the maze is guaranteed
// solvable regardless of the extra loops; a BFS afterward finds a shortest
// route from the kiosk to the exit for the guide-arrow overlay, which -
// like before - only ever gets drawn once kioskSolved is true.
//
// Room physical coordinates are offset by one cell from the grid's edges on
// every side (RoomPhysRow starts at 1, not 0; the array is sized with a
// spare column/row past the far side too) so that a room carved right at
// the edge of the room grid still has a genuine CELL_WALL cube between it
// and the edge of the level - otherwise the player could stand in an edge
// room and look straight out into the void past where the grid ends.
#define ROOM_GRID 8

typedef struct { int rr, rc; } RoomPos;

static int RoomPhysCol(int rc) { return 4 + rc * 2; }
static int RoomPhysRow(int rr) { return 1 + rr * 2; }

static void BuildRoomMaze(Maze *m, Color wallColor, Color floorColor) {
    const int width = 4 + ROOM_GRID * 2 + 1;   // foyer + rooms + 1-cell east margin
    const int height = 1 + ROOM_GRID * 2 + 1;  // 1-cell north margin + rooms + 1-cell south margin
    memset(m, 0, sizeof(*m));
    m->width = width;
    m->height = height;
    m->wallColor = wallColor;
    m->floorColor = floorColor;
    m->startYawDeg = 90.0f;

    for (int r = 0; r < height; r++)
        for (int c = 0; c < width; c++)
            m->cells[r][c] = CELL_WALL;

    // Recursive-backtracker maze. Room (0, entryRow) sits right past the
    // gate; the far corner room is the level exit.
    const RoomPos entry = { ROOM_GRID / 2, 0 };
    const RoomPos exit = { ROOM_GRID - 1, ROOM_GRID - 1 };

    // Foyer: start -> kiosk -> locked gate, all carved due east, aligned
    // with the entry room's row so the gate opens directly into it.
    int foyerRow = RoomPhysRow(entry.rr);
    m->startCol = 1; m->startRow = foyerRow;
    m->cells[foyerRow][1] = CELL_FLOOR;
    m->cells[foyerRow][2] = CELL_FLOOR;
    m->kioskCol = 2; m->kioskRow = foyerRow;
    m->cells[foyerRow][3] = CELL_BLOCK;

    bool visited[ROOM_GRID][ROOM_GRID];
    memset(visited, 0, sizeof(visited));

    RoomPos stack[ROOM_GRID * ROOM_GRID];
    int stackLen = 0;
    visited[entry.rr][entry.rc] = true;
    m->cells[RoomPhysRow(entry.rr)][RoomPhysCol(entry.rc)] = CELL_FLOOR;
    stack[stackLen++] = entry;

    const int dr[4] = { -1, 1, 0, 0 };
    const int dc[4] = { 0, 0, -1, 1 };

    while (stackLen > 0) {
        RoomPos cur = stack[stackLen - 1];
        RoomPos candidates[4];
        int candidateCount = 0;
        for (int i = 0; i < 4; i++) {
            int rr = cur.rr + dr[i];
            int rc = cur.rc + dc[i];
            if (rr < 0 || rr >= ROOM_GRID || rc < 0 || rc >= ROOM_GRID) continue;
            if (visited[rr][rc]) continue;
            candidates[candidateCount++] = (RoomPos){ rr, rc };
        }

        if (candidateCount == 0) {
            stackLen--;
            continue;
        }

        RoomPos next = candidates[GetRandomValue(0, candidateCount - 1)];
        int connRow = (RoomPhysRow(cur.rr) + RoomPhysRow(next.rr)) / 2;
        int connCol = (RoomPhysCol(cur.rc) + RoomPhysCol(next.rc)) / 2;
        m->cells[connRow][connCol] = CELL_FLOOR;
        m->cells[RoomPhysRow(next.rr)][RoomPhysCol(next.rc)] = CELL_FLOOR;
        visited[next.rr][next.rc] = true;
        stack[stackLen++] = next;
    }

    // Braiding pass: the backtracker alone produces a "perfect" maze - a
    // spanning tree with exactly one route between any two rooms, all dead
    // ends and no loops. Knock down a handful of extra walls between
    // already-adjacent rooms to add genuine loops/alternate routes on top
    // of that, without ever touching reachability (every room the tree
    // already connects stays connected either way).
    for (int rr = 0; rr < ROOM_GRID; rr++) {
        for (int rc = 0; rc < ROOM_GRID; rc++) {
            if (rr + 1 < ROOM_GRID && GetRandomValue(0, 99) < 16) {
                int connRow = (RoomPhysRow(rr) + RoomPhysRow(rr + 1)) / 2;
                int connCol = RoomPhysCol(rc);
                m->cells[connRow][connCol] = CELL_FLOOR;
            }
            if (rc + 1 < ROOM_GRID && GetRandomValue(0, 99) < 16) {
                int connRow = RoomPhysRow(rr);
                int connCol = (RoomPhysCol(rc) + RoomPhysCol(rc + 1)) / 2;
                m->cells[connRow][connCol] = CELL_FLOOR;
            }
        }
    }

    m->exitCol = RoomPhysCol(exit.rc);
    m->exitRow = RoomPhysRow(exit.rr);

    // BFS from entry to exit over the carved connections to find the guide path.
    bool bfsVisited[ROOM_GRID][ROOM_GRID];
    RoomPos parent[ROOM_GRID][ROOM_GRID];
    memset(bfsVisited, 0, sizeof(bfsVisited));
    for (int rr = 0; rr < ROOM_GRID; rr++)
        for (int rc = 0; rc < ROOM_GRID; rc++)
            parent[rr][rc] = (RoomPos){ -1, -1 };

    RoomPos queue[ROOM_GRID * ROOM_GRID];
    int qHead = 0, qTail = 0;
    bfsVisited[entry.rr][entry.rc] = true;
    queue[qTail++] = entry;

    while (qHead < qTail) {
        RoomPos cur = queue[qHead++];
        for (int i = 0; i < 4; i++) {
            int rr = cur.rr + dr[i];
            int rc = cur.rc + dc[i];
            if (rr < 0 || rr >= ROOM_GRID || rc < 0 || rc >= ROOM_GRID) continue;
            if (bfsVisited[rr][rc]) continue;
            int connRow = (RoomPhysRow(cur.rr) + RoomPhysRow(rr)) / 2;
            int connCol = (RoomPhysCol(cur.rc) + RoomPhysCol(rc)) / 2;
            if (m->cells[connRow][connCol] != CELL_FLOOR) continue;
            bfsVisited[rr][rc] = true;
            parent[rr][rc] = cur;
            queue[qTail++] = (RoomPos){ rr, rc };
        }
    }

    RoomPos roomPath[ROOM_GRID * ROOM_GRID];
    int roomPathLen = 0;
    RoomPos cur = exit;
    while (cur.rr != entry.rr || cur.rc != entry.rc) {
        roomPath[roomPathLen++] = cur;
        cur = parent[cur.rr][cur.rc];
    }
    roomPath[roomPathLen++] = entry;
    for (int i = 0; i < roomPathLen / 2; i++) {
        RoomPos t = roomPath[i];
        roomPath[i] = roomPath[roomPathLen - 1 - i];
        roomPath[roomPathLen - 1 - i] = t;
    }

    m->pathLen = 0;
    for (int i = 0; i < roomPathLen; i++) {
        int physCol = RoomPhysCol(roomPath[i].rc);
        int physRow = RoomPhysRow(roomPath[i].rr);
        if (i > 0) {
            int prevCol = RoomPhysCol(roomPath[i - 1].rc);
            int prevRow = RoomPhysRow(roomPath[i - 1].rr);
            m->pathCol[m->pathLen] = (prevCol + physCol) / 2;
            m->pathRow[m->pathLen] = (prevRow + physRow) / 2;
            m->pathLen++;
        }
        m->pathCol[m->pathLen] = physCol;
        m->pathRow[m->pathLen] = physRow;
        m->pathLen++;
    }
}

void Maze_InitLevel(Maze *m, int levelIndex) {
    switch (levelIndex) {
        case 1: {
            // The Corridor: a single straight hallway leading to the
            // terminal at the far end, matching the script ("straight
            // hallway... at the end, he solves a BRS trivia challenge").
            CarveStep seq[] = {
                { DIR_S, 10 },  // approach: the hallway itself
                { DIR_S, 1 },   // kiosk, at the end of the hallway
                { DIR_S, 1 },   // gate
                { DIR_S, 2 },   // short stretch beyond the door to the exit
            };
            BuildMaze(m, 3, 17, 1, 1, 180.0f,
                      (Color){ 70, 78, 110, 255 }, (Color){ 35, 40, 60, 255 },
                      seq, 4, 1);
            break;
        }
        case 2: {
            // The Engine Room Maze.
            BuildRoomMaze(m, (Color){ 60, 90, 110, 255 }, (Color){ 25, 45, 55, 255 });
            break;
        }
        case 3: {
            // The Cargo Bay Maze.
            BuildRoomMaze(m, (Color){ 110, 90, 60, 255 }, (Color){ 55, 45, 25, 255 });
            break;
        }
        case 4: {
            // The Command Deck Maze.
            BuildRoomMaze(m, (Color){ 90, 70, 110, 255 }, (Color){ 40, 30, 55, 255 });
            break;
        }
        case 5:
        default: {
            // The Cockpit Maze.
            BuildRoomMaze(m, (Color){ 110, 60, 70, 255 }, (Color){ 50, 25, 30, 255 });
            break;
        }
    }
}

Vector3 Maze_CellToWorld(const Maze *m, int col, int row) {
    float originX = -((float)m->width * CELL_SIZE) / 2.0f;
    float originZ = -((float)m->height * CELL_SIZE) / 2.0f;
    return (Vector3){
        originX + (float)col * CELL_SIZE + CELL_SIZE / 2.0f,
        0.0f,
        originZ + (float)row * CELL_SIZE + CELL_SIZE / 2.0f
    };
}

static bool WorldToCell(const Maze *m, float x, float z, int *outCol, int *outRow) {
    float originX = -((float)m->width * CELL_SIZE) / 2.0f;
    float originZ = -((float)m->height * CELL_SIZE) / 2.0f;
    int col = (int)floorf((x - originX) / CELL_SIZE);
    int row = (int)floorf((z - originZ) / CELL_SIZE);
    if (col < 0 || col >= m->width || row < 0 || row >= m->height) return false;
    *outCol = col;
    *outRow = row;
    return true;
}

static bool PointWalkable(const Maze *m, float x, float z, bool kioskSolved) {
    int col, row;
    if (!WorldToCell(m, x, z, &col, &row)) return false;
    CellType t = m->cells[row][col];
    if (t == CELL_WALL) return false;
    if (t == CELL_BLOCK) return kioskSolved;
    return true;
}

bool Maze_CircleWalkable(const Maze *m, float x, float z, float radius, bool kioskSolved) {
    if (!PointWalkable(m, x, z, kioskSolved)) return false;
    if (!PointWalkable(m, x + radius, z, kioskSolved)) return false;
    if (!PointWalkable(m, x - radius, z, kioskSolved)) return false;
    if (!PointWalkable(m, x, z + radius, kioskSolved)) return false;
    if (!PointWalkable(m, x, z - radius, kioskSolved)) return false;
    return true;
}

bool Maze_NearKiosk(const Maze *m, Vector3 pos, float radius) {
    Vector3 k = Maze_CellToWorld(m, m->kioskCol, m->kioskRow);
    float dx = pos.x - k.x;
    float dz = pos.z - k.z;
    return (dx * dx + dz * dz) <= radius * radius;
}

bool Maze_ReachedExit(const Maze *m, Vector3 pos, float radius) {
    Vector3 e = Maze_CellToWorld(m, m->exitCol, m->exitRow);
    float dx = pos.x - e.x;
    float dz = pos.z - e.z;
    return (dx * dx + dz * dz) <= radius * radius;
}

static void DrawStars(void) {
    static Vector3 stars[300];
    static bool initialized = false;
    if (!initialized) {
        unsigned int seed = 4242u;
        for (int i = 0; i < 300; i++) {
            seed = seed * 1103515245u + 12345u;
            float a = ((float)((seed >> 8) % 36000)) / 100.0f * DEG2RAD;
            seed = seed * 1103515245u + 12345u;
            float b = (((float)((seed >> 8) % 18000)) / 100.0f - 90.0f) * DEG2RAD;
            float r = 90.0f;
            stars[i] = (Vector3){ r * cosf(b) * cosf(a), fabsf(r * sinf(b)) + 10.0f, r * cosf(b) * sinf(a) };
        }
        initialized = true;
    }
    for (int i = 0; i < 300; i++) {
        DrawPoint3D(stars[i], (Color){ 210, 215, 235, 255 });
    }
}

static void DrawFloorArrow(Vector3 center, float yawDeg, Color color) {
    rlPushMatrix();
    rlTranslatef(center.x, 0.05f, center.z);
    rlRotatef(yawDeg, 0.0f, 1.0f, 0.0f);
    DrawTriangle3D((Vector3){ -0.5f, 0, -0.6f }, (Vector3){ 0.5f, 0, -0.6f }, (Vector3){ 0.0f, 0, 0.6f }, color);
    rlPopMatrix();
}

void Maze_Draw(const Maze *m, bool kioskSolved) {
    DrawStars();

    float w = (float)m->width * CELL_SIZE;
    float h = (float)m->height * CELL_SIZE;
    DrawPlane((Vector3){ 0, 0, 0 }, (Vector2){ w, h }, m->floorColor);
    DrawPlane((Vector3){ 0, WALL_HEIGHT, 0 }, (Vector2){ w, h }, (Color){ 15, 18, 35, 255 });

    for (int r = 0; r < m->height; r++) {
        for (int c = 0; c < m->width; c++) {
            CellType t = m->cells[r][c];
            if (t == CELL_WALL) {
                Vector3 p = Maze_CellToWorld(m, c, r);
                p.y = WALL_HEIGHT / 2.0f;
                DrawCube(p, CELL_SIZE, WALL_HEIGHT, CELL_SIZE, m->wallColor);
            } else if (t == CELL_BLOCK && !kioskSolved) {
                Vector3 p = Maze_CellToWorld(m, c, r);
                p.y = WALL_HEIGHT / 2.0f;
                DrawCube(p, CELL_SIZE, WALL_HEIGHT, CELL_SIZE, (Color){ 200, 70, 70, 180 });
                DrawCubeWires(p, CELL_SIZE, WALL_HEIGHT, CELL_SIZE, (Color){ 255, 150, 150, 255 });
            }
        }
    }

    // Kiosk pillar.
    Vector3 kioskPos = Maze_CellToWorld(m, m->kioskCol, m->kioskRow);
    DrawCube((Vector3){ kioskPos.x, 0.8f, kioskPos.z }, 0.8f, 1.6f, 0.8f, (Color){ 124, 108, 246, 255 });
    DrawCubeWires((Vector3){ kioskPos.x, 0.8f, kioskPos.z }, 0.8f, 1.6f, 0.8f, (Color){ 200, 190, 255, 255 });

    // Exit marker.
    Vector3 exitPos = Maze_CellToWorld(m, m->exitCol, m->exitRow);
    DrawCube((Vector3){ exitPos.x, 0.05f, exitPos.z }, CELL_SIZE * 0.6f, 0.1f, CELL_SIZE * 0.6f, (Color){ 90, 200, 140, kioskSolved ? 220 : 60 });

    if (kioskSolved) {
        for (int i = 0; i + 1 < m->pathLen; i++) {
            Vector3 a = Maze_CellToWorld(m, m->pathCol[i], m->pathRow[i]);
            int dcol = m->pathCol[i + 1] - m->pathCol[i];
            int drow = m->pathRow[i + 1] - m->pathRow[i];
            float yaw = 0.0f;
            if (dcol == 1) yaw = 90.0f;
            else if (dcol == -1) yaw = 270.0f;
            else if (drow == 1) yaw = 0.0f;
            else if (drow == -1) yaw = 180.0f;
            DrawFloorArrow(a, yaw, (Color){ 120, 230, 255, 230 });
        }
    }
}
