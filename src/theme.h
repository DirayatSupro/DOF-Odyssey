#ifndef THEME_H
#define THEME_H

#include "raylib.h"
#include <stdbool.h>

extern const Color COL_BG;
extern const Color COL_PANEL;
extern const Color COL_PANEL_BORDER;
extern const Color COL_ACCENT;
extern const Color COL_ACCENT_DIM;
extern const Color COL_TEXT;
extern const Color COL_TEXT_DIM;
extern const Color COL_DANGER;
extern const Color COL_SUCCESS;

// Draws a deterministic scattered starfield/dot-grid background within the
// given screen rectangle, matching the look of the provided menu mockups.
void DrawSpaceBackdrop(int screenW, int screenH);

// A rounded, bordered button with centered label. Returns true on the frame
// it is clicked. `accent` draws it in the highlighted purple style used for
// the primary action; otherwise it uses the dim panel style. `danger` tints
// text/border red (used for Quit).
bool UiButton(Rectangle rect, const char *label, bool accent, bool danger);

void DrawCenteredText(const char *text, int centerX, int y, int fontSize, Color color);

#endif
