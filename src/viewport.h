#ifndef VIEWPORT_H
#define VIEWPORT_H

#include "raylib.h"

// The whole game is drawn to a fixed-size virtual canvas at this resolution,
// then that canvas is scaled (preserving aspect ratio, letterboxed) to fill
// whatever the real window/monitor size is. Every screen's layout math was
// written against this resolution - use these instead of GetScreenWidth()/
// GetScreenHeight() anywhere inside the drawing code, since those report the
// real (resizable/fullscreen) window size, not the virtual canvas.
#define VIRTUAL_WIDTH 1280
#define VIRTUAL_HEIGHT 720

typedef struct {
    Rectangle dest; // where the virtual canvas lands on the real window
    float scale;
} Viewport;

// Computes the current letterboxed destination rect + scale factor from the
// real window size. Cheap; safe to call every frame / every click check.
Viewport Viewport_Compute(void);

// Mouse position remapped from real window coordinates into virtual-canvas
// coordinates - use this instead of GetMousePosition() for any UI hit
// testing while the game is drawn through the virtual canvas.
Vector2 Viewport_GetMouse(void);

#endif
