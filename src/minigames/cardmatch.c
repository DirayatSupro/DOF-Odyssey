#include "minigame.h"
#include "assets.h"
#include "theme.h"
#include "viewport.h"
#include <math.h>
#include <string.h>

#define CM_COLS 7
#define CM_ROWS 2
#define CM_COUNT (CM_COLS * CM_ROWS)
#define CM_ICONS 7

typedef struct {
    int icon;
    bool matched;
    bool faceUp;
} Card;

static Card cards[CM_COUNT];
static int firstIndex, secondIndex;
static float resolveTimer;
static int matchesFound;
static Rectangle cardRects[CM_COUNT];
static const char *iconNames[CM_ICONS] = { "Nebula", "Galaxy", "Star", "Blackhole", "Rocket", "Astronaut", "Asteroid" };

static void CardMatchInit(void) {
    int deck[CM_COUNT];
    for (int i = 0; i < CM_ICONS; i++) { deck[i * 2] = i; deck[i * 2 + 1] = i; }
    for (int i = CM_COUNT - 1; i > 0; i--) {
        int j = GetRandomValue(0, i);
        int tmp = deck[i]; deck[i] = deck[j]; deck[j] = tmp;
    }
    for (int i = 0; i < CM_COUNT; i++) {
        cards[i].icon = deck[i];
        cards[i].matched = false;
        cards[i].faceUp = false;
    }

    firstIndex = -1;
    secondIndex = -1;
    resolveTimer = 0.0f;
    matchesFound = 0;

    float cardW = 120, cardH = 150, gap = 12;
    float totalW = CM_COLS * cardW + (CM_COLS - 1) * gap;
    float totalH = CM_ROWS * cardH + (CM_ROWS - 1) * gap;
    float startX = VIRTUAL_WIDTH / 2.0f - totalW / 2.0f;
    float startY = 130;
    (void)totalH;

    for (int r = 0; r < CM_ROWS; r++) {
        for (int c = 0; c < CM_COLS; c++) {
            int idx = r * CM_COLS + c;
            cardRects[idx] = (Rectangle){ startX + c * (cardW + gap), startY + r * (cardH + gap), cardW, cardH };
        }
    }
}

static MinigameStatus CardMatchUpdate(float dt) {
    if (resolveTimer > 0.0f) {
        resolveTimer -= dt;
        if (resolveTimer <= 0.0f) {
            if (cards[firstIndex].icon == cards[secondIndex].icon) {
                cards[firstIndex].matched = true;
                cards[secondIndex].matched = true;
                matchesFound++;
                Audio_Play(audio.cardMatch);
            } else {
                cards[firstIndex].faceUp = false;
                cards[secondIndex].faceUp = false;
            }
            firstIndex = -1;
            secondIndex = -1;
        }
        return matchesFound == CM_ICONS ? MG_SUCCESS : MG_RUNNING;
    }

    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && secondIndex == -1) {
        Vector2 mouse = Viewport_GetMouse();
        for (int i = 0; i < CM_COUNT; i++) {
            if (cards[i].matched || cards[i].faceUp) continue;
            if (CheckCollisionPointRec(mouse, cardRects[i])) {
                cards[i].faceUp = true;
                if (firstIndex == -1) {
                    firstIndex = i;
                } else {
                    secondIndex = i;
                    resolveTimer = 0.6f;
                }
                break;
            }
        }
    }

    return matchesFound == CM_ICONS ? MG_SUCCESS : MG_RUNNING;
}

static void DrawStar4(Vector2 c, float outerR, float innerR, float rotRad, Color color) {
    Vector2 pts[8];
    for (int i = 0; i < 8; i++) {
        float ang = rotRad + i * (PI / 4.0f);
        float r = (i % 2 == 0) ? outerR : innerR;
        pts[i] = (Vector2){ c.x + cosf(ang) * r, c.y + sinf(ang) * r };
    }
    for (int i = 0; i < 8; i++) {
        DrawTriangle(pts[i], c, pts[(i + 1) % 8], color);
    }
}

static void DrawIcon(int icon, Vector2 center, float size) {
    float t = (float)GetTime();
    switch (icon) {
        case 0: { // Nebula: 3 flat circles, pulse scale
            float pulse = 1.0f + sinf(t * 2.5f) * 0.08f;
            DrawCircleV((Vector2){ center.x - size * 0.15f, center.y - size * 0.05f }, size * 0.32f * pulse, Fade((Color){ 168, 130, 230, 255 }, 0.8f));
            DrawCircleV((Vector2){ center.x + size * 0.12f, center.y - size * 0.12f }, size * 0.28f * pulse, Fade((Color){ 120, 160, 220, 255 }, 0.75f));
            DrawCircleV((Vector2){ center.x, center.y + size * 0.14f }, size * 0.3f * pulse, Fade((Color){ 220, 120, 170, 255 }, 0.8f));
            break;
        }
        case 1: { // Galaxy: disk + 2 dots orbit center
            DrawCircleV(center, size * 0.38f, (Color){ 40, 60, 110, 255 });
            DrawCircleV(center, size * 0.14f, (Color){ 240, 240, 235, 255 });
            float angle = t * 1.8f;
            Vector2 d1 = { center.x + cosf(angle) * size * 0.34f, center.y + sinf(angle) * size * 0.34f };
            Vector2 d2 = { center.x - cosf(angle) * size * 0.30f, center.y - sinf(angle) * size * 0.30f };
            DrawCircleV(d1, size * 0.06f, (Color){ 130, 200, 240, 255 });
            DrawCircleV(d2, size * 0.05f, (Color){ 240, 240, 240, 255 });
            break;
        }
        case 2: { // Star: one polygon, pulse scale
            float pulse = 1.0f + sinf(t * 3.0f) * 0.12f;
            DrawStar4(center, size * 0.42f * pulse, size * 0.14f * pulse, 0.0f, (Color){ 232, 195, 90, 255 });
            break;
        }
        case 3: { // Blackhole: ring + core, dot orbits
            DrawCircleV(center, size * 0.24f, BLACK);
            DrawRing(center, size * 0.3f, size * 0.38f, 0, 360, 32, (Color){ 230, 140, 60, 255 });
            float angle = t * 2.4f;
            Vector2 d = { center.x + cosf(angle) * size * 0.38f, center.y + sinf(angle) * size * 0.38f };
            DrawCircleV(d, size * 0.06f, (Color){ 250, 210, 100, 255 });
            break;
        }
        case 4: { // Rocket: triangle + rect, bob + flame flicker
            float bob = sinf(t * 3.0f) * size * 0.03f;
            Vector2 top = { center.x, center.y - size * 0.42f + bob };
            Vector2 bl = { center.x - size * 0.18f, center.y - size * 0.05f + bob };
            Vector2 br = { center.x + size * 0.18f, center.y - size * 0.05f + bob };
            DrawTriangle(top, bl, br, (Color){ 210, 214, 224, 255 });
            DrawRectangle((int)(center.x - size * 0.14f), (int)(center.y - size * 0.05f + bob), (int)(size * 0.28f), (int)(size * 0.34f), (Color){ 235, 100, 60, 255 });
            float flicker = 0.6f + 0.4f * sinf(t * 25.0f);
            DrawTriangle((Vector2){ center.x - size * 0.1f, center.y + size * 0.29f + bob },
                         (Vector2){ center.x + size * 0.1f, center.y + size * 0.29f + bob },
                         (Vector2){ center.x, center.y + (0.29f + 0.22f * flicker) * size + bob },
                         (Color){ 250, 170, 60, 255 });
            break;
        }
        case 5: { // Astronaut: 2 circles + rect, float bob
            float bob = sinf(t * 2.0f) * size * 0.04f;
            DrawCircleV((Vector2){ center.x, center.y - size * 0.05f + bob }, size * 0.34f, (Color){ 235, 236, 240, 255 });
            DrawCircleV((Vector2){ center.x, center.y - size * 0.05f + bob }, size * 0.2f, (Color){ 25, 30, 50, 255 });
            DrawRectangleRounded((Rectangle){ center.x - size * 0.16f, center.y + size * 0.22f + bob, size * 0.32f, size * 0.16f }, 0.5f, 6, (Color){ 235, 236, 240, 255 });
            break;
        }
        case 6:
        default: { // Asteroid: hexagon, continuous rotate
            float rotDeg = t * 30.0f;
            DrawPoly(center, 6, size * 0.36f, rotDeg, (Color){ 150, 130, 100, 255 });
            float rad = rotDeg * DEG2RAD;
            Vector2 crater1 = { center.x + cosf(rad + 0.6f) * size * 0.12f, center.y + sinf(rad + 0.6f) * size * 0.12f };
            Vector2 crater2 = { center.x + cosf(rad - 1.1f) * size * 0.16f, center.y + sinf(rad - 1.1f) * size * 0.16f };
            DrawCircleV(crater1, size * 0.06f, (Color){ 110, 92, 70, 255 });
            DrawCircleV(crater2, size * 0.05f, (Color){ 110, 92, 70, 255 });
            break;
        }
    }
}

static void CardMatchDraw(void) {
    int sw = VIRTUAL_WIDTH;
    DrawCenteredText("COSMIC MATCH", sw / 2, 24, 30, COL_TEXT);
    DrawCenteredText(TextFormat("Matches %d/%d", matchesFound, CM_ICONS), sw / 2, 60, 18, COL_TEXT_DIM);

    for (int i = 0; i < CM_COUNT; i++) {
        Rectangle r = cardRects[i];
        if (cards[i].matched) {
            DrawRectangleRounded(r, 0.15f, 6, (Color){ 20, 40, 30, 255 });
            DrawRectangleRoundedLinesEx(r, 0.15f, 6, 2.0f, COL_SUCCESS);
            DrawIcon(cards[i].icon, (Vector2){ r.x + r.width / 2, r.y + r.height / 2 }, r.width);
        } else if (cards[i].faceUp) {
            DrawRectangleRounded(r, 0.15f, 6, COL_PANEL);
            DrawRectangleRoundedLinesEx(r, 0.15f, 6, 2.0f, COL_ACCENT);
            DrawIcon(cards[i].icon, (Vector2){ r.x + r.width / 2, r.y + r.height / 2 }, r.width);
        } else {
            DrawRectangleRounded(r, 0.15f, 6, (Color){ 26, 30, 55, 255 });
            DrawRectangleRoundedLinesEx(r, 0.15f, 6, 1.5f, COL_PANEL_BORDER);
            float t = (float)GetTime() * 1.5f + i;
            DrawPoly((Vector2){ r.x + r.width / 2, r.y + r.height / 2 }, 4, 10 + sinf(t) * 2, 45, (Color){ 90, 95, 130, 255 });
        }
    }

    int legendY = (int)(cardRects[CM_COUNT - 1].y + cardRects[CM_COUNT - 1].height + 30);
    DrawCenteredText("Collection", sw / 2, legendY, 18, COL_TEXT_DIM);
    for (int i = 0; i < CM_ICONS; i++) {
        int x = sw / 2 - (CM_ICONS * 90) / 2 + i * 90 + 45;
        int y = legendY + 40;
        bool found = false;
        for (int c = 0; c < CM_COUNT; c++) if (cards[c].icon == i && cards[c].matched) found = true;
        DrawCircle(x, y, 24, found ? (Color){ 30, 45, 40, 255 } : (Color){ 20, 22, 40, 255 });
        if (found) DrawIcon(i, (Vector2){ (float)x, (float)y }, 40);
        DrawCenteredText(iconNames[i], x, y + 32, 12, found ? COL_SUCCESS : COL_TEXT_DIM);
    }
}

Minigame CardMatch_GetInterface(void) {
    Minigame m;
    m.Init = CardMatchInit;
    m.Update = CardMatchUpdate;
    m.Draw = CardMatchDraw;
    return m;
}
