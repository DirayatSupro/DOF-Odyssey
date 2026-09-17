#include "ui.h"
#include "assets.h"
#include "theme.h"
#include "viewport.h"
#include "level.h"
#include <string.h>

static Rectangle FracRect(float l, float t, float r, float b) {
    int sw = VIRTUAL_WIDTH;
    int sh = VIRTUAL_HEIGHT;
    return (Rectangle){ l * sw, t * sh, (r - l) * sw, (b - t) * sh };
}

static void DrawFullscreenTexture(Texture2D tex) {
    Rectangle src = { 0, 0, (float)tex.width, (float)tex.height };
    Rectangle dst = { 0, 0, (float)VIRTUAL_WIDTH, (float)VIRTUAL_HEIGHT };
    DrawTexturePro(tex, src, dst, (Vector2){ 0, 0 }, 0.0f, WHITE);
}

void UI_DrawStartMenu(void) {
    DrawFullscreenTexture(textures.start);

    Rectangle newGame   = FracRect(0.359f, 0.419f, 0.640f, 0.499f);
    Rectangle continueG = FracRect(0.359f, 0.522f, 0.640f, 0.602f);
    Rectangle leaderb   = FracRect(0.359f, 0.621f, 0.640f, 0.701f);
    Rectangle settings  = FracRect(0.359f, 0.721f, 0.640f, 0.801f);

    Vector2 mouse = Viewport_GetMouse();
    bool clicked = IsMouseButtonPressed(MOUSE_LEFT_BUTTON);

    if (clicked && CheckCollisionPointRec(mouse, newGame)) {
        app.screen = SCREEN_GAME_SCRIPT;
    } else if (clicked && CheckCollisionPointRec(mouse, continueG)) {
        if (app.save.hasSave) {
            Level_LoadFromSave();
            app.screen = SCREEN_PLAYING;
            Level_OnScreenActivated();
        }
    } else if (clicked && CheckCollisionPointRec(mouse, leaderb)) {
        app.screen = SCREEN_LEADERBOARD;
    } else if (clicked && CheckCollisionPointRec(mouse, settings)) {
        app.screen = SCREEN_SETTINGS;
    }

    if (!app.save.hasSave) {
        DrawText("(no saved game)", (int)continueG.x, (int)(continueG.y + continueG.height + 4), 14, (Color){ 150, 155, 180, 200 });
    }
}

void UI_DrawSettings(AppScreen backTarget) {
    DrawSpaceBackdrop(VIRTUAL_WIDTH, VIRTUAL_HEIGHT);

    int sw = VIRTUAL_WIDTH;
    int sh = VIRTUAL_HEIGHT;
    Rectangle panel = { sw / 2.0f - 260, sh / 2.0f - 180, 520, 360 };
    DrawRectangleRounded(panel, 0.08f, 8, COL_PANEL);
    DrawRectangleRoundedLinesEx(panel, 0.08f, 8, 2.0f, COL_PANEL_BORDER);

    DrawCenteredText("SETTINGS", sw / 2, (int)panel.y + 30, 32, COL_TEXT);

    Rectangle muteBtn = { panel.x + 40, panel.y + 120, panel.width - 80, 56 };
    const char *label = app.settings.muted ? "Sound: MUTED (click to unmute)" : "Sound: ON (click to mute)";
    if (UiButton(muteBtn, label, !app.settings.muted, false)) {
        Settings_ToggleMute();
    }

    Rectangle backBtn = { panel.x + 40, panel.y + 240, panel.width - 80, 56 };
    if (UiButton(backBtn, "Back", false, false)) {
        app.screen = backTarget;
        if (backTarget == SCREEN_PLAYING) Level_OnScreenActivated();
    }
}

void UI_DrawLeaderboard(void) {
    DrawSpaceBackdrop(VIRTUAL_WIDTH, VIRTUAL_HEIGHT);

    int sw = VIRTUAL_WIDTH;
    int sh = VIRTUAL_HEIGHT;
    Rectangle panel = { sw / 2.0f - 260, sh / 2.0f - 260, 520, 520 };
    DrawRectangleRounded(panel, 0.06f, 8, COL_PANEL);
    DrawRectangleRoundedLinesEx(panel, 0.06f, 8, 2.0f, COL_PANEL_BORDER);

    DrawCenteredText("LEADERBOARD", sw / 2, (int)panel.y + 26, 30, COL_TEXT);
    DrawCenteredText("Fastest cockpit rescues", sw / 2, (int)panel.y + 62, 16, COL_TEXT_DIM);

    if (app.leaderboardCount == 0) {
        DrawCenteredText("No completed runs yet.", sw / 2, (int)panel.y + 160, 18, COL_TEXT_DIM);
    } else {
        for (int i = 0; i < app.leaderboardCount; i++) {
            int y = (int)panel.y + 100 + i * 34;
            int minutes = (int)app.leaderboard[i].seconds / 60;
            int seconds = (int)app.leaderboard[i].seconds % 60;
            DrawText(TextFormat("%2d. %s", i + 1, app.leaderboard[i].name), (int)panel.x + 30, y, 18, COL_TEXT);
            DrawText(TextFormat("%02d:%02d", minutes, seconds), (int)panel.x + (int)panel.width - 90, y, 18, COL_ACCENT);
        }
    }

    Rectangle backBtn = { panel.x + 40, panel.y + panel.height - 76, panel.width - 80, 52 };
    if (UiButton(backBtn, "Back", false, false)) {
        app.screen = SCREEN_START_MENU;
    }
}

// A section header is drawn in the accent color; every other line is a
// plain body/rule line. Kept as one flat array (rather than nested per
// level) so the whole thing can just be scrolled through top to bottom.
typedef struct { const char *text; bool isHeader; } ScriptLine;

static const ScriptLine scriptLines[] = {
    { "After a catastrophic failure aboard the deep-space research vessel Odyssey,", false },
    { "Rocky awakens trapped in the lower decks. The ship's main computer has", false },
    { "locked down navigation control, revoking all physical degrees of freedom.", false },
    { "To survive and reach the cockpit, Rocky must navigate 5 dangerous sectors,", false },
    { "regaining physical movement step-by-step.", false },
    { "", false },
    { "SECTOR 1 - THE CORRIDOR", true },
    { "A straight hallway. Rocky can only move Forwards.", false },
    { "Reach the console at the end and solve the BUET Robotics Society (BRS)", false },
    { "trivia challenge to unlock the 2nd Degree of Freedom: Moving Backwards.", false },
    { "Basic Rules:", false },
    { "  - Answer the Question.", false },
    { "", false },
    { "SECTOR 2 - THE ENGINE ROOM MAZE", true },
    { "Move forward and turn left/right to find the console.", false },
    { "Solve the space-themed Color Connect puzzle (Orbit Flow) - connect all", false },
    { "8 planets and Earth's Moon with matching color paths - to unlock the", false },
    { "3rd Degree of Freedom: Moving Left.", false },
    { "Basic Rules:", false },
    { "  - Connect: draw a line between matching colored dots.", false },
    { "  - Don't Cross: lines cannot cross or overlap.", false },
    { "  - Fill the Grid: cover every empty square to clear the puzzle.", false },
    { "", false },
    { "SECTOR 3 - THE CARGO BAY MAZE", true },
    { "A more complex maze using forward, backward, and turning movement.", false },
    { "Solve the 3x3 sliding tile puzzle (the ship's schematic) to unlock the", false },
    { "4th Degree of Freedom: Moving Right.", false },
    { "Basic Rules:", false },
    { "  - Slide: move tiles into the empty space to rearrange them.", false },
    { "  - Order: arrange the pieces 1 through 8, left to right, top to bottom.", false },
    { "  - Complete: leave the empty space in the bottom-right corner.", false },
    { "", false },
    { "SECTOR 4 - THE COMMAND DECK MAZE", true },
    { "Full ground movement is active - forward, backward, left, and right.", false },
    { "Win the Cosmic Match memory game to unlock the 5th Degree of Freedom:", false },
    { "using the Spacebar.", false },
    { "Basic Rules:", false },
    { "  - Flip: turn over any two cards to reveal their faces.", false },
    { "  - Match: matching cards stay face-up; mismatches flip back down.", false },
    { "  - Clear: match all seven pairs to clear the board.", false },
    { "", false },
    { "SECTOR 5 - THE COCKPIT MAZE", true },
    { "Navigate the final stretch using every Degree of Freedom you've regained.", false },
    { "Defeat Star Defender to override the lockdown and take the Key to the", false },
    { "Cockpit.", false },
    { "Basic Rules:", false },
    { "  - Move & Shoot: strafe your ship and fire upward with Spacebar.", false },
    { "  - Dodge: avoid enemy fire and use the shield for cover.", false },
    { "  - Defend: destroy every invader before one reaches the bottom.", false },
    { "", false },
    { "Press E near a console to begin that sector's challenge. Without", false },
    { "solving it, the way forward stays sealed.", false },
};

static float scriptScroll = 0.0f;

void UI_DrawGameScript(bool preGame) {
    DrawSpaceBackdrop(VIRTUAL_WIDTH, VIRTUAL_HEIGHT);

    int sw = VIRTUAL_WIDTH;
    int sh = VIRTUAL_HEIGHT;
    Rectangle panel = { sw / 2.0f - 430, 30, 860, (float)sh - 60 };
    DrawRectangleRounded(panel, 0.035f, 8, COL_PANEL);
    DrawRectangleRoundedLinesEx(panel, 0.035f, 8, 2.0f, COL_PANEL_BORDER);

    DrawCenteredText("GAME SCRIPT", sw / 2, (int)panel.y + 18, 28, COL_TEXT);
    if (preGame) {
        DrawCenteredText("Read the mission briefing, then begin sector 1.", sw / 2, (int)panel.y + 50, 15, COL_TEXT_DIM);
    }

    float contentTop = panel.y + (preGame ? 82.0f : 64.0f);
    float contentBottom = panel.y + panel.height - 70;
    float contentHeight = contentBottom - contentTop;
    int lineCount = (int)(sizeof(scriptLines) / sizeof(scriptLines[0]));
    const int lineHeight = 26;
    float totalHeight = (float)(lineCount * lineHeight);
    float maxScroll = totalHeight - contentHeight;
    if (maxScroll < 0.0f) maxScroll = 0.0f;

    scriptScroll -= GetMouseWheelMove() * 36.0f;
    if (scriptScroll < 0.0f) scriptScroll = 0.0f;
    if (scriptScroll > maxScroll) scriptScroll = maxScroll;

    BeginScissorMode((int)panel.x, (int)contentTop, (int)panel.width, (int)contentHeight);
    for (int i = 0; i < lineCount; i++) {
        float y = contentTop + i * lineHeight - scriptScroll;
        if (y < contentTop - lineHeight || y > contentBottom) continue;
        Color c = scriptLines[i].isHeader ? COL_ACCENT : COL_TEXT_SOFT;
        int fontSize = scriptLines[i].isHeader ? 21 : 19;
        DrawText(scriptLines[i].text, (int)panel.x + 34, (int)y, fontSize, c);
    }
    EndScissorMode();

    if (maxScroll > 0.0f) {
        float trackH = contentHeight;
        float thumbH = trackH * (contentHeight / totalHeight);
        if (thumbH < 24.0f) thumbH = 24.0f;
        float thumbY = contentTop + (trackH - thumbH) * (scriptScroll / maxScroll);
        Rectangle track = { panel.x + panel.width - 16, contentTop, 7, trackH };
        Rectangle thumb = { panel.x + panel.width - 16, thumbY, 7, thumbH };
        DrawRectangleRounded(track, 0.5f, 4, (Color){ 30, 34, 60, 255 });
        DrawRectangleRounded(thumb, 0.5f, 4, COL_ACCENT_DIM);
        DrawCenteredText("scroll for more", sw / 2, (int)(contentBottom + 8), 14, COL_TEXT_DIM);
    }

    Rectangle actionBtn = { panel.x + panel.width / 2 - 150, panel.y + panel.height - 56, 300, 44 };
    if (preGame) {
        if (UiButton(actionBtn, "Begin Sector 1", true, false)) {
            Level_StartNew("");
            app.screen = SCREEN_PLAYING;
            Level_OnScreenActivated();
        }
    } else {
        if (UiButton(actionBtn, "Back", false, false)) {
            app.screen = SCREEN_PAUSED;
        }
    }
}

void UI_DrawPauseMenu(void) {
    DrawFullscreenTexture(textures.pause);

    Rectangle resume  = FracRect(0.365f, 0.363f, 0.635f, 0.444f);
    Rectangle restart = FracRect(0.365f, 0.451f, 0.635f, 0.531f);
    Rectangle settings= FracRect(0.365f, 0.539f, 0.635f, 0.617f);
    Rectangle script  = FracRect(0.365f, 0.625f, 0.635f, 0.703f);
    Rectangle quit    = FracRect(0.365f, 0.709f, 0.635f, 0.788f);

    Vector2 mouse = Viewport_GetMouse();
    bool clicked = IsMouseButtonPressed(MOUSE_LEFT_BUTTON);

    if (clicked && CheckCollisionPointRec(mouse, resume)) {
        app.screen = SCREEN_PLAYING;
        Level_OnScreenActivated();
    } else if (clicked && CheckCollisionPointRec(mouse, restart)) {
        app.screen = SCREEN_PAUSE_RESTART_CONFIRM;
    } else if (clicked && CheckCollisionPointRec(mouse, settings)) {
        app.screen = SCREEN_PAUSE_SETTINGS;
    } else if (clicked && CheckCollisionPointRec(mouse, script)) {
        app.screen = SCREEN_PAUSE_SCRIPT;
    } else if (clicked && CheckCollisionPointRec(mouse, quit)) {
        app.running = false;
    }
}

void UI_DrawPauseRestartConfirm(void) {
    DrawSpaceBackdrop(VIRTUAL_WIDTH, VIRTUAL_HEIGHT);

    int sw = VIRTUAL_WIDTH;
    int sh = VIRTUAL_HEIGHT;
    Rectangle panel = { sw / 2.0f - 260, sh / 2.0f - 170, 520, 340 };
    DrawRectangleRounded(panel, 0.08f, 8, COL_PANEL);
    DrawRectangleRoundedLinesEx(panel, 0.08f, 8, 2.0f, COL_PANEL_BORDER);

    DrawCenteredText("RESTART", sw / 2, (int)panel.y + 26, 28, COL_TEXT);
    DrawCenteredText("What would you like to restart?", sw / 2, (int)panel.y + 66, 16, COL_TEXT_DIM);

    Rectangle restartLevel = { panel.x + 40, panel.y + 110, panel.width - 80, 54 };
    Rectangle restartGame  = { panel.x + 40, panel.y + 176, panel.width - 80, 54 };
    Rectangle cancel       = { panel.x + 40, panel.y + 250, panel.width - 80, 46 };

    if (UiButton(restartLevel, "Restart This Level", true, false)) {
        Level_RestartCurrentLevel();
        app.screen = SCREEN_PLAYING;
        Level_OnScreenActivated();
    } else if (UiButton(restartGame, "Restart Whole Game", false, false)) {
        Level_RestartWholeGame();
        app.screen = SCREEN_PLAYING;
        Level_OnScreenActivated();
    } else if (UiButton(cancel, "Cancel", false, false)) {
        app.screen = SCREEN_PAUSED;
    }
}

