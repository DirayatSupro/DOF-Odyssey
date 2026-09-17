#include "theme.h"
#include "viewport.h"
#include <stdlib.h>
#include <string.h>

const Color COL_BG            = (Color){ 10, 13, 30, 255 };
const Color COL_PANEL         = (Color){ 20, 24, 51, 255 };
const Color COL_PANEL_BORDER  = (Color){ 42, 48, 80, 255 };
const Color COL_ACCENT        = (Color){ 124, 108, 246, 255 };
const Color COL_ACCENT_DIM    = (Color){ 90, 78, 190, 255 };
const Color COL_TEXT          = (Color){ 240, 238, 230, 255 };
const Color COL_TEXT_SOFT     = (Color){ 202, 208, 226, 255 };
const Color COL_TEXT_DIM      = (Color){ 136, 145, 176, 255 };
const Color COL_DANGER        = (Color){ 235, 100, 100, 255 };
const Color COL_SUCCESS       = (Color){ 90, 200, 140, 255 };

void DrawSpaceBackdrop(int screenW, int screenH) {
    ClearBackground(COL_BG);
    static Vector2 dots[400];
    static bool initialized = false;
    if (!initialized) {
        unsigned int seed = 1337;
        for (int i = 0; i < 400; i++) {
            seed = seed * 1103515245u + 12345u;
            float x = (float)((seed >> 8) % 10000) / 10000.0f;
            seed = seed * 1103515245u + 12345u;
            float y = (float)((seed >> 8) % 10000) / 10000.0f;
            dots[i] = (Vector2){ x, y };
        }
        initialized = true;
    }
    for (int i = 0; i < 400; i++) {
        int x = (int)(dots[i].x * screenW);
        int y = (int)(dots[i].y * screenH);
        DrawPixel(x, y, (Color){ 120, 130, 165, 160 });
    }
}

bool UiButton(Rectangle rect, const char *label, bool accent, bool danger) {
    Vector2 mouse = Viewport_GetMouse();
    bool hovered = CheckCollisionPointRec(mouse, rect);

    Color bg = accent ? COL_ACCENT : COL_PANEL;
    if (hovered && !accent) bg = (Color){ 30, 35, 65, 255 };
    if (hovered && accent) bg = (Color){ 140, 125, 250, 255 };

    Color border = danger ? COL_DANGER : COL_PANEL_BORDER;
    Color text = danger ? COL_DANGER : (accent ? COL_TEXT : COL_TEXT);

    DrawRectangleRounded(rect, 0.25f, 8, bg);
    DrawRectangleRoundedLinesEx(rect, 0.25f, 8, 1.5f, border);

    int fontSize = 22;
    int tw = MeasureText(label, fontSize);
    DrawText(label, (int)(rect.x + rect.width / 2 - tw / 2), (int)(rect.y + rect.height / 2 - fontSize / 2), fontSize, text);

    return hovered && IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
}

void DrawCenteredText(const char *text, int centerX, int y, int fontSize, Color color) {
    int tw = MeasureText(text, fontSize);
    DrawText(text, centerX - tw / 2, y, fontSize, color);
}
