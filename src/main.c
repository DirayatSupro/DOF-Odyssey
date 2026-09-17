#include "raylib.h"
#include "app.h"
#include "assets.h"
#include "theme.h"
#include "ui.h"
#include "level.h"
#include "respath.h"
#include <time.h>

int main(void) {
    const int screenWidth = 1280;
    const int screenHeight = 720;

    InitWindow(screenWidth, screenHeight, "Degrees of Freedom: Lost on the Odyssey");
    SetTargetFPS(60);
    SetExitKey(KEY_NULL); // ESC is used for the in-game pause menu, not window close
    SetRandomSeed((unsigned int)time(NULL)); // different maze layout / puzzle shuffle every run

    App_Init();
    Assets_LoadAll();

    while (app.running && !WindowShouldClose()) {
        float dt = GetFrameTime();
        Audio_UpdateMusic();

        bool musicScreen = (app.screen == SCREEN_START_MENU || app.screen == SCREEN_SETTINGS || app.screen == SCREEN_LEADERBOARD);
        Audio_SetMusicPlaying(musicScreen);

        if (IsKeyPressed(KEY_ESCAPE)) {
            if (app.screen == SCREEN_PLAYING) {
                app.screen = SCREEN_PAUSED;
                Level_OnScreenDeactivated();
            } else if (app.screen == SCREEN_PAUSED) {
                app.screen = SCREEN_PLAYING;
                Level_OnScreenActivated();
            }
        }

        if (app.screen == SCREEN_PLAYING) {
            Level_Update(dt);
        }

        BeginDrawing();

        switch (app.screen) {
            case SCREEN_START_MENU:
                UI_DrawStartMenu();
                break;
            case SCREEN_SETTINGS:
                UI_DrawSettings(SCREEN_START_MENU);
                break;
            case SCREEN_LEADERBOARD:
                UI_DrawLeaderboard();
                break;
            case SCREEN_GAME_SCRIPT:
                UI_DrawGameScript();
                break;
            case SCREEN_PLAYING:
                Level_Draw();
                break;
            case SCREEN_PAUSED:
                UI_DrawPauseMenu();
                break;
            case SCREEN_PAUSE_RESTART_CONFIRM:
                UI_DrawPauseRestartConfirm();
                break;
            case SCREEN_PAUSE_SETTINGS:
                UI_DrawSettings(SCREEN_PAUSED);
                break;
            case SCREEN_PAUSE_SCRIPT:
                UI_DrawGameScript();
                break;
        }

        EndDrawing();

        if (IsKeyPressed(KEY_F12)) {
            TakeScreenshot(ResolveWritablePath("saves/screenshot.png"));
        }
    }

    Assets_UnloadAll();
    App_Shutdown();
    CloseWindow();
    return 0;
}
