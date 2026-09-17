#ifndef MAZE_H
#define MAZE_H

#include "raylib.h"
#include <stdbool.h>

#define MAZE_MAX_W 26
#define MAZE_MAX_H 26
#define MAZE_MAX_PATH (MAZE_MAX_W * MAZE_MAX_H)

#define CELL_SIZE 4.0f
#define WALL_HEIGHT 3.2f
#define EYE_HEIGHT 1.7f
#define PLAYER_RADIUS 0.35f

typedef enum {
    CELL_WALL,
    CELL_FLOOR,
    CELL_BLOCK   // acts as a wall until the level's kiosk minigame is solved
} CellType;

typedef struct {
    int width, height;
    CellType cells[MAZE_MAX_H][MAZE_MAX_W];

    int startCol, startRow;
    float startYawDeg;

    int kioskCol, kioskRow;
    int exitCol, exitRow;

    // Ordered cells from just past the gate to the exit, used both to know
    // the exit is reachable and to draw guiding arrow decals once solved.
    int pathCol[MAZE_MAX_PATH];
    int pathRow[MAZE_MAX_PATH];
    int pathLen;

    Color wallColor;
    Color floorColor;
} Maze;

// Builds the hand-authored layout for level 1..5 into *m.
void Maze_InitLevel(Maze *m, int levelIndex);

Vector3 Maze_CellToWorld(const Maze *m, int col, int row);

bool Maze_CircleWalkable(const Maze *m, float x, float z, float radius, bool kioskSolved);
bool Maze_NearKiosk(const Maze *m, Vector3 pos, float radius);
bool Maze_ReachedExit(const Maze *m, Vector3 pos, float radius);

void Maze_Draw(const Maze *m, bool kioskSolved, Vector3 cameraPos);

#endif
