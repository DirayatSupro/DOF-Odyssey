#include "viewport.h"

Viewport Viewport_Compute(void) {
    float actualW = (float)GetScreenWidth();
    float actualH = (float)GetScreenHeight();

    float scale = actualW / VIRTUAL_WIDTH;
    float scaleH = actualH / VIRTUAL_HEIGHT;
    if (scaleH < scale) scale = scaleH;
    if (scale <= 0.0f) scale = 1.0f;

    float destW = VIRTUAL_WIDTH * scale;
    float destH = VIRTUAL_HEIGHT * scale;

    Viewport vp;
    vp.scale = scale;
    vp.dest = (Rectangle){ (actualW - destW) / 2.0f, (actualH - destH) / 2.0f, destW, destH };
    return vp;
}

Vector2 Viewport_GetMouse(void) {
    Viewport vp = Viewport_Compute();
    Vector2 m = GetMousePosition();
    return (Vector2){
        (m.x - vp.dest.x) / vp.scale,
        (m.y - vp.dest.y) / vp.scale
    };
}
