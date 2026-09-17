#ifndef UI_H
#define UI_H

#include "app.h"

// Each function draws its screen for the current frame and mutates
// app.screen (and related state) in response to clicks. Called once per
// frame from main.c based on the current app.screen value.
void UI_DrawStartMenu(void);
void UI_DrawSettings(AppScreen backTarget);
void UI_DrawLeaderboard(void);
void UI_DrawGameScript(void);
void UI_DrawPauseMenu(void);
void UI_DrawPauseRestartConfirm(void);

#endif
