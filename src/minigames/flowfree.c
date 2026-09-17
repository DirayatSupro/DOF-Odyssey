#include "minigame.h"
#include "assets.h"
#include "theme.h"
#include <string.h>
#include <math.h>

#define FF_N 6
#define FF_COLORS 9
#define FF_MAX_PATH (FF_N * FF_N)

typedef struct {
    int r0, c0;   // endpoint A
    int r1, c1;   // endpoint B
    Color color;
    const char *label;
    bool connected;
    int pathR[FF_MAX_PATH];
    int pathC[FF_MAX_PATH];
    int pathLen;
} FlowColor;

static FlowColor colorsData[FF_COLORS];
static int owner[FF_N][FF_N];       // -1 = empty, else color index
static bool dragging;
static int dragColor;
static int lastTouchedColor;
static int filledCount;

static Rectangle gridRect;
static float cellSize;

static void ResetColorPath(int idx) {
    FlowColor *c = &colorsData[idx];
    for (int i = 0; i < c->pathLen; i++) {
        owner[c->pathR[i]][c->pathC[i]] = -1;
    }
    c->pathLen = 0;
    c->connected = false;
}

static void RecountFilled(void) {
    filledCount = 0;
    for (int r = 0; r < FF_N; r++)
        for (int c = 0; c < FF_N; c++)
            if (owner[r][c] != -1) filledCount++;
}

// Generates a random reference solution for the board: a Hamiltonian path
// that visits all 36 cells exactly once, split into 9 consecutive 4-cell
// chunks (one per color). Because each chunk is a run of consecutive cells
// on a single simple path, it is automatically itself a valid connected,
// non-branching path - so every color is guaranteed a solvable route and the
// full board is guaranteed coverable, without ever repeating the same
// blocky shape twice. A randomized start + randomized neighbor order means
// a different layout is generated every time the puzzle is played.
static bool ffVisited[FF_N][FF_N];
static int ffPath[FF_N * FF_N];
static long ffStepBudget;

static bool ExtendHamPath(int r, int c, int count) {
    if (--ffStepBudget <= 0) return false;

    ffPath[count] = r * FF_N + c;
    ffVisited[r][c] = true;
    if (count + 1 == FF_N * FF_N) return true;

    int dr[4] = { -1, 1, 0, 0 };
    int dc[4] = { 0, 0, -1, 1 };
    for (int i = 3; i > 0; i--) {
        int j = GetRandomValue(0, i);
        int tr = dr[i]; dr[i] = dr[j]; dr[j] = tr;
        int tc = dc[i]; dc[i] = dc[j]; dc[j] = tc;
    }

    for (int i = 0; i < 4; i++) {
        int nr = r + dr[i], nc = c + dc[i];
        if (nr < 0 || nr >= FF_N || nc < 0 || nc >= FF_N) continue;
        if (ffVisited[nr][nc]) continue;
        if (ExtendHamPath(nr, nc, count + 1)) return true;
    }

    ffVisited[r][c] = false;
    return false;
}

static void GenerateHamiltonianPath(int outPath[FF_N * FF_N]) {
    for (int attempt = 0; attempt < 10; attempt++) {
        memset(ffVisited, 0, sizeof(ffVisited));
        ffStepBudget = 500000;
        int sr = GetRandomValue(0, FF_N - 1);
        int sc = GetRandomValue(0, FF_N - 1);
        if (ExtendHamPath(sr, sc, 0)) {
            memcpy(outPath, ffPath, sizeof(ffPath));
            return;
        }
    }

    // Deterministic fallback (always valid) in the unlikely event the
    // randomized search above doesn't land one within its step budget.
    int idx = 0;
    for (int r = 0; r < FF_N; r++) {
        if (r % 2 == 0) {
            for (int c = 0; c < FF_N; c++) outPath[idx++] = r * FF_N + c;
        } else {
            for (int c = FF_N - 1; c >= 0; c--) outPath[idx++] = r * FF_N + c;
        }
    }
}

static void FlowFreeInit(void) {
    const Color palette[FF_COLORS] = {
        (Color){ 232, 121, 58, 255 },   // Mercury - orange
        (Color){ 240, 111, 161, 255 },  // Venus - pink
        (Color){ 63, 166, 91, 255 },    // Earth - green
        (Color){ 169, 172, 184, 255 },  // Moon - gray
        (Color){ 196, 59, 59, 255 },    // Mars - red
        (Color){ 232, 184, 75, 255 },   // Jupiter - yellow
        (Color){ 240, 237, 228, 255 },  // Saturn - white
        (Color){ 111, 198, 232, 255 },  // Neptune - light blue
        (Color){ 74, 127, 214, 255 },   // Uranus - blue
    };
    const char *labels[FF_COLORS] = {
        "Mercury", "Venus", "Earth", "Moon", "Mars", "Jupiter", "Saturn", "Neptune", "Uranus"
    };

    for (int r = 0; r < FF_N; r++)
        for (int c = 0; c < FF_N; c++)
            owner[r][c] = -1;

    int solution[FF_N * FF_N];
    GenerateHamiltonianPath(solution);

    for (int i = 0; i < FF_COLORS; i++) {
        int startIdx = solution[i * 4 + 0];
        int endIdx = solution[i * 4 + 3];
        FlowColor *fc = &colorsData[i];
        fc->r0 = startIdx / FF_N;
        fc->c0 = startIdx % FF_N;
        fc->r1 = endIdx / FF_N;
        fc->c1 = endIdx % FF_N;
        fc->color = palette[i];
        fc->label = labels[i];
        fc->connected = false;
        fc->pathLen = 0;
    }

    dragging = false;
    dragColor = -1;
    lastTouchedColor = -1;
    filledCount = 0;

    int sw = GetScreenWidth();
    float size = 460.0f;
    gridRect = (Rectangle){ sw / 2.0f - size / 2.0f, 90, size, size };
    cellSize = size / FF_N;
}

static bool CellToScreenValid(Vector2 mouse, int *outR, int *outC) {
    if (!CheckCollisionPointRec(mouse, gridRect)) return false;
    int c = (int)((mouse.x - gridRect.x) / cellSize);
    int r = (int)((mouse.y - gridRect.y) / cellSize);
    if (r < 0 || r >= FF_N || c < 0 || c >= FF_N) return false;
    *outR = r;
    *outC = c;
    return true;
}

static int EndpointColorAt(int r, int c, bool *isFirstEndpoint) {
    for (int i = 0; i < FF_COLORS; i++) {
        if (colorsData[i].r0 == r && colorsData[i].c0 == c) { if (isFirstEndpoint) *isFirstEndpoint = true; return i; }
        if (colorsData[i].r1 == r && colorsData[i].c1 == c) { if (isFirstEndpoint) *isFirstEndpoint = false; return i; }
    }
    return -1;
}

static bool Adjacent(int r0, int c0, int r1, int c1) {
    int dr = r0 - r1; if (dr < 0) dr = -dr;
    int dc = c0 - c1; if (dc < 0) dc = -dc;
    return (dr + dc) == 1;
}

static void PushCell(int colorIdx, int r, int c) {
    FlowColor *fc = &colorsData[colorIdx];
    fc->pathR[fc->pathLen] = r;
    fc->pathC[fc->pathLen] = c;
    fc->pathLen++;
    owner[r][c] = colorIdx;
}

static MinigameStatus FlowFreeUpdate(float dt) {
    (void)dt;
    Vector2 mouse = GetMousePosition();
    int r, c;
    bool onGrid = CellToScreenValid(mouse, &r, &c);

    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && onGrid) {
        bool isFirst;
        int colorIdx = EndpointColorAt(r, c, &isFirst);
        if (colorIdx != -1) {
            ResetColorPath(colorIdx);
            RecountFilled();
            PushCell(colorIdx, r, c);
            dragging = true;
            dragColor = colorIdx;
            lastTouchedColor = colorIdx;
            RecountFilled();
        }
    } else if (dragging && IsMouseButtonDown(MOUSE_LEFT_BUTTON) && onGrid) {
        FlowColor *fc = &colorsData[dragColor];
        int lastR = fc->pathR[fc->pathLen - 1];
        int lastC = fc->pathC[fc->pathLen - 1];

        if (r != lastR || c != lastC) {
            if (fc->pathLen >= 2 && fc->pathR[fc->pathLen - 2] == r && fc->pathC[fc->pathLen - 2] == c) {
                // backtrack
                owner[lastR][lastC] = -1;
                fc->pathLen--;
                fc->connected = false;
            } else if (Adjacent(lastR, lastC, r, c)) {
                bool isFirst;
                int epColor = EndpointColorAt(r, c, &isFirst);
                if (epColor == dragColor) {
                    // reached an endpoint of this color
                    bool isOwnStart = (r == fc->pathR[0] && c == fc->pathC[0]);
                    if (!isOwnStart && owner[r][c] == -1) {
                        PushCell(dragColor, r, c);
                        fc->connected = true;
                        dragging = false;
                        Audio_Play(audio.colorConnected);
                    }
                } else if (epColor != -1) {
                    // another color's endpoint - can't pass through
                } else if (owner[r][c] == -1) {
                    PushCell(dragColor, r, c);
                }
            }
            RecountFilled();
        }
    }

    if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
        dragging = false;
    }

    bool allConnected = true;
    for (int i = 0; i < FF_COLORS; i++) {
        if (!colorsData[i].connected) { allConnected = false; break; }
    }

    if (allConnected && filledCount == FF_N * FF_N) {
        return MG_SUCCESS;
    }

    return MG_RUNNING;
}

static void DrawPlanetDot(Vector2 center, float radius, Color color, const char *label) {
    (void)label;
    DrawCircleV(center, radius, color);
    Color darker = ColorBrightness(color, -0.35f);
    DrawCircle((int)(center.x - radius * 0.25f), (int)(center.y + radius * 0.15f), radius * 0.16f, darker);
    DrawCircle((int)(center.x + radius * 0.2f), (int)(center.y + radius * 0.3f), radius * 0.12f, darker);
}

static void FlowFreeDraw(void) {
    int sw = GetScreenWidth();

    DrawCenteredText("ORBIT FLOW", sw / 2, 28, 30, COL_TEXT);
    DrawCenteredText("Connect each planet pair and fill every cell", sw / 2, 64, 16, COL_TEXT_DIM);

    DrawRectangleRec(gridRect, (Color){ 14, 17, 36, 255 });
    DrawRectangleLinesEx(gridRect, 2.0f, COL_PANEL_BORDER);

    for (int i = 1; i < FF_N; i++) {
        DrawLine((int)(gridRect.x + i * cellSize), (int)gridRect.y, (int)(gridRect.x + i * cellSize), (int)(gridRect.y + gridRect.height), (Color){ 40, 46, 74, 255 });
        DrawLine((int)gridRect.x, (int)(gridRect.y + i * cellSize), (int)(gridRect.x + gridRect.width), (int)(gridRect.y + i * cellSize), (Color){ 40, 46, 74, 255 });
    }

    for (int r = 0; r < FF_N; r++) {
        for (int c = 0; c < FF_N; c++) {
            int idx = owner[r][c];
            if (idx == -1) continue;
            Rectangle cellRect = { gridRect.x + c * cellSize + 6, gridRect.y + r * cellSize + 6, cellSize - 12, cellSize - 12 };
            DrawRectangleRounded(cellRect, 0.4f, 6, Fade(colorsData[idx].color, 0.55f));
        }
    }

    for (int i = 0; i < FF_COLORS; i++) {
        FlowColor *fc = &colorsData[i];
        Vector2 p0 = { gridRect.x + fc->c0 * cellSize + cellSize / 2, gridRect.y + fc->r0 * cellSize + cellSize / 2 };
        Vector2 p1 = { gridRect.x + fc->c1 * cellSize + cellSize / 2, gridRect.y + fc->r1 * cellSize + cellSize / 2 };
        DrawPlanetDot(p0, cellSize * 0.32f, fc->color, fc->label);
        DrawPlanetDot(p1, cellSize * 0.32f, fc->color, fc->label);
        if (strcmp(fc->label, "Saturn") == 0) {
            DrawEllipseLines((int)p0.x, (int)p0.y, cellSize * 0.5f, cellSize * 0.18f, (Color){ 180, 180, 190, 200 });
            DrawEllipseLines((int)p1.x, (int)p1.y, cellSize * 0.5f, cellSize * 0.18f, (Color){ 180, 180, 190, 200 });
        }
    }

    int legendY = (int)(gridRect.y + gridRect.height + 20);
    for (int i = 0; i < FF_COLORS; i++) {
        int col = i % 5;
        int row = i / 5;
        int x = (int)gridRect.x + col * 130;
        int y = legendY + row * 26;
        DrawCircle(x + 8, y + 8, 7, colorsData[i].color);
        DrawText(colorsData[i].label, x + 22, y, 16, colorsData[i].connected ? COL_SUCCESS : COL_TEXT_DIM);
    }

    Rectangle resetBtn = { gridRect.x + gridRect.width - 200, (float)(legendY + 56), 200, 40 };
    Rectangle undoBtn = { gridRect.x, (float)(legendY + 56), 200, 40 };

    if (UiButton(undoBtn, "Undo", false, false)) {
        if (lastTouchedColor != -1 && colorsData[lastTouchedColor].pathLen > 1) {
            FlowColor *fc = &colorsData[lastTouchedColor];
            int r = fc->pathR[fc->pathLen - 1];
            int c = fc->pathC[fc->pathLen - 1];
            owner[r][c] = -1;
            fc->pathLen--;
            fc->connected = false;
            RecountFilled();
        }
    }
    if (UiButton(resetBtn, "Reset", false, true)) {
        FlowFreeInit();
    }

    DrawText("Drag from a dot through empty cells to the matching dot.", (int)gridRect.x, legendY + 106, 14, COL_TEXT_DIM);
}

Minigame FlowFree_GetInterface(void) {
    Minigame m;
    m.Init = FlowFreeInit;
    m.Update = FlowFreeUpdate;
    m.Draw = FlowFreeDraw;
    return m;
}
