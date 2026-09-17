#include "minigame.h"
#include "assets.h"
#include "theme.h"
#include <string.h>
#include <stdlib.h>

#define SL_N 3
#define SL_COUNT (SL_N * SL_N)

static int board[SL_COUNT]; // tile id 1..8 in each slot, 0 = blank
static int blankIndex;
static Rectangle boardRect;
static float tileW, tileH;
static bool wonAnnounced;

static void SlidingInit(void) {
    for (int i = 0; i < SL_COUNT - 1; i++) board[i] = i + 1;
    board[SL_COUNT - 1] = 0;
    blankIndex = SL_COUNT - 1;

    for (int i = 0; i < 150; i++) {
        int r = blankIndex / SL_N;
        int c = blankIndex % SL_N;
        int options[4];
        int optionCount = 0;
        if (r > 0) options[optionCount++] = blankIndex - SL_N;
        if (r < SL_N - 1) options[optionCount++] = blankIndex + SL_N;
        if (c > 0) options[optionCount++] = blankIndex - 1;
        if (c < SL_N - 1) options[optionCount++] = blankIndex + 1;

        int pick = options[GetRandomValue(0, optionCount - 1)];
        board[blankIndex] = board[pick];
        board[pick] = 0;
        blankIndex = pick;
    }

    int sw = GetScreenWidth();
    float bw = 400.0f;
    float bh = bw * ((float)textures.slidingtile.height / (float)textures.slidingtile.width);
    boardRect = (Rectangle){ sw / 2.0f - bw / 2.0f, 96, bw, bh };
    tileW = bw / SL_N;
    tileH = bh / SL_N;
    wonAnnounced = false;
}

static bool IsSolved(void) {
    for (int i = 0; i < SL_COUNT - 1; i++) {
        if (board[i] != i + 1) return false;
    }
    return board[SL_COUNT - 1] == 0;
}

static void TrySlide(int index) {
    int r = index / SL_N, c = index % SL_N;
    int br = blankIndex / SL_N, bc = blankIndex % SL_N;
    int dr = r - br; if (dr < 0) dr = -dr;
    int dc = c - bc; if (dc < 0) dc = -dc;
    if (dr + dc == 1) {
        board[blankIndex] = board[index];
        board[index] = 0;
        blankIndex = index;
        Audio_Play(audio.slidingMove);
    }
}

static MinigameStatus SlidingUpdate(float dt) {
    (void)dt;
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        Vector2 mouse = GetMousePosition();
        if (CheckCollisionPointRec(mouse, boardRect)) {
            int c = (int)((mouse.x - boardRect.x) / tileW);
            int r = (int)((mouse.y - boardRect.y) / tileH);
            if (r >= 0 && r < SL_N && c >= 0 && c < SL_N) {
                TrySlide(r * SL_N + c);
            }
        }
    }

    if (IsKeyPressed(KEY_UP)) { int t = blankIndex + SL_N; if (t < SL_COUNT) TrySlide(t); }
    if (IsKeyPressed(KEY_DOWN)) { int t = blankIndex - SL_N; if (t >= 0) TrySlide(t); }
    if (IsKeyPressed(KEY_LEFT)) { int t = blankIndex + 1; if (t % SL_N != 0) TrySlide(t); }
    if (IsKeyPressed(KEY_RIGHT)) { int t = blankIndex % SL_N != 0 ? blankIndex - 1 : -1; if (t >= 0) TrySlide(t); }

    if (IsSolved()) {
        if (!wonAnnounced) wonAnnounced = true;
        return MG_SUCCESS;
    }
    return MG_RUNNING;
}

static void SlidingDraw(void) {
    int sw = GetScreenWidth();
    DrawCenteredText("STARSHIP SCHEMATIC", sw / 2, 24, 30, COL_TEXT);
    DrawCenteredText("Slide the tiles into order: 1-8, blank in the bottom-right", sw / 2, 60, 16, COL_TEXT_DIM);

    DrawRectangleRec(boardRect, (Color){ 10, 12, 26, 255 });

    for (int r = 0; r < SL_N; r++) {
        for (int c = 0; c < SL_N; c++) {
            int idx = r * SL_N + c;
            int tile = board[idx];
            Rectangle dest = { boardRect.x + c * tileW + 2, boardRect.y + r * tileH + 2, tileW - 4, tileH - 4 };
            if (tile == 0) {
                DrawRectangleRec(dest, (Color){ 18, 21, 40, 255 });
                continue;
            }

            int homeRow = (tile - 1) / SL_N;
            int homeCol = (tile - 1) % SL_N;
            float srcW = (float)textures.slidingtile.width / SL_N;
            float srcH = (float)textures.slidingtile.height / SL_N;
            Rectangle src = { homeCol * srcW, homeRow * srcH, srcW, srcH };

            DrawTexturePro(textures.slidingtile, src, dest, (Vector2){ 0, 0 }, 0.0f, WHITE);
            DrawRectangleLinesEx(dest, 2.0f, (Color){ 124, 108, 246, 200 });

            Rectangle badge = { dest.x + dest.width - 30, dest.y + dest.height - 30, 26, 26 };
            DrawRectangleRounded(badge, 0.3f, 6, (Color){ 10, 12, 26, 220 });
            DrawText(TextFormat("%d", tile), (int)(badge.x + 8), (int)(badge.y + 4), 18, COL_TEXT);
        }
    }

    if (IsSolved()) {
        DrawCenteredText("Schematic aligned!", sw / 2, (int)(boardRect.y + boardRect.height + 24), 22, COL_SUCCESS);
    }
}

Minigame Sliding_GetInterface(void) {
    Minigame m;
    m.Init = SlidingInit;
    m.Update = SlidingUpdate;
    m.Draw = SlidingDraw;
    return m;
}
