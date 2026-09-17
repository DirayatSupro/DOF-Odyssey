#include "raylib.h"
#include "app.h"
#include "assets.h"
#include "theme.h"
#include "ui.h"
#include "level.h"
#include "respath.h"
#include "viewport.h"
#include <time.h>

#if defined(PLATFORM_WEB)
#include <emscripten/emscripten.h>
#endif

static RenderTexture2D canvas;

static void UpdateDrawFrame(void) {
    float dt = GetFrameTime();
    Audio_UpdateMusic();

    bool musicScreen = (app.screen == SCREEN_START_MENU || app.screen == SCREEN_SETTINGS || app.screen == SCREEN_LEADERBOARD || app.screen == SCREEN_GAME_SCRIPT
                        || (app.screen == SCREEN_PLAYING && levelState.ending));
    Audio_SetMusicPlaying(musicScreen);

    if (IsKeyPressed(KEY_F11)) {
        ToggleBorderlessWindowed();
    }

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

    // Draw everything to the fixed-size virtual canvas first...
    BeginTextureMode(canvas);

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
            UI_DrawGameScript(true);
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
            UI_DrawGameScript(false);
            break;
    }

    EndTextureMode();

    // ...then scale+letterbox that canvas onto the real (resizable,
    // possibly fullscreen) window, preserving aspect ratio.
    BeginDrawing();
    ClearBackground(BLACK);
    Viewport vp = Viewport_Compute();
    Rectangle src = { 0, 0, (float)canvas.texture.width, -(float)canvas.texture.height };
    DrawTexturePro(canvas.texture, src, vp.dest, (Vector2){ 0, 0 }, 0.0f, WHITE);
    EndDrawing();

#if !defined(PLATFORM_WEB)
    if (IsKeyPressed(KEY_F12)) {
        TakeScreenshot(ResolveWritablePath("saves/screenshot.png"));
    }
#endif
}

int main(void) {
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(VIRTUAL_WIDTH, VIRTUAL_HEIGHT, "Degrees of Freedom: Lost on the Odyssey");
    SetWindowMinSize(640, 360);
    SetTargetFPS(60);
    SetExitKey(KEY_NULL); // ESC is used for the in-game pause menu, not window close
    SetRandomSeed((unsigned int)time(NULL)); // different maze layout / puzzle shuffle every run

    App_Init();
    Assets_LoadAll();

    canvas = LoadRenderTexture(VIRTUAL_WIDTH, VIRTUAL_HEIGHT);
    SetTextureFilter(canvas.texture, TEXTURE_FILTER_BILINEAR);

#if defined(PLATFORM_WEB)
    // Browsers don't allow a blocking loop - the runtime drives frames via
    // its own callback instead. simulate_infinite_loop=1 keeps everything
    // (including our own while-loop-shaped app.running flag) working the
    // same as the desktop build from the game's point of view.
    emscripten_set_main_loop(UpdateDrawFrame, 0, 1);
#else
    while (app.running && !WindowShouldClose()) {
        UpdateDrawFrame();
    }
#endif

    UnloadRenderTexture(canvas);
    Assets_UnloadAll();
    App_Shutdown();
    CloseWindow();
    return 0;
}
