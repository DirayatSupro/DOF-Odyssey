#include "assets.h"
#include "app.h"
#include "respath.h"
#include <math.h>

GameTextures textures;
GameAudio audio;
Texture2D signs[SIGN_COUNT];

static Texture2D LoadTex(const char *rel) {
    return LoadTexture(ResolvePath(rel));
}

static Sound LoadSnd(const char *rel) {
    return LoadSound(ResolvePath(rel));
}

static RenderTexture2D MakeSignCanvas(void) {
    RenderTexture2D rt = LoadRenderTexture(256, 128);
    SetTextureFilter(rt.texture, TEXTURE_FILTER_BILINEAR);
    return rt;
}

// Render textures are stored bottom-up (OpenGL framebuffer convention), so
// reading one back and handing it straight to a manual textured quad would
// come out upside down. Flipping it once into a normal Texture2D here means
// every other piece of code that draws a sign can just use the same plain,
// unambiguous top-is-top texture-coordinate convention as any other loaded
// image in this project.
static Texture2D FinishSignCanvas(RenderTexture2D rt) {
    EndTextureMode();
    Image img = LoadImageFromTexture(rt.texture);
    ImageFlipVertical(&img);
    Texture2D tex = LoadTextureFromImage(img);
    SetTextureFilter(tex, TEXTURE_FILTER_BILINEAR);
    UnloadImage(img);
    UnloadRenderTexture(rt);
    return tex;
}

static Texture2D MakeDesignationSign(const char *code, const char *sub) {
    const int W = 256, H = 128;
    RenderTexture2D rt = MakeSignCanvas();
    BeginTextureMode(rt);
        ClearBackground((Color){ 40, 42, 48, 255 });
        DrawRectangleLinesEx((Rectangle){ 6, 6, W - 12, H - 12 }, 2, (Color){ 150, 155, 165, 255 });
        int dw = MeasureText(code, 46);
        DrawText(code, W / 2 - dw / 2, H / 2 - 30, 46, (Color){ 225, 228, 235, 255 });
        int sw2 = MeasureText(sub, 13);
        DrawText(sub, W / 2 - sw2 / 2, H - 28, 13, (Color){ 150, 155, 165, 255 });
    return FinishSignCanvas(rt);
}

// Small placards and technical diagrams mounted on the maze walls, drawn
// once here rather than shipped as image files - stenciled labels, hazard
// markings, and instrument-panel-style diagrams so the corridors read as a
// built ship interior instead of plain colored blocks.
static void GenerateSigns(void) {
    const int W = 256, H = 128;
    RenderTexture2D rt;

    rt = MakeSignCanvas();
    BeginTextureMode(rt);
        ClearBackground((Color){ 12, 12, 14, 255 });
        for (int i = -14; i < W; i += 20) {
            DrawRectangle(i, 0, 10, 14, (Color){ 235, 190, 40, 255 });
            DrawRectangle(i, H - 14, 10, 14, (Color){ 235, 190, 40, 255 });
        }
        DrawRectangleLinesEx((Rectangle){ 4, 4, W - 8, H - 8 }, 3, (Color){ 220, 70, 60, 255 });
        int cw = MeasureText("CAUTION", 34);
        DrawText("CAUTION", W / 2 - cw / 2, H / 2 - 17, 34, (Color){ 245, 235, 210, 255 });
    signs[SIGN_CAUTION] = FinishSignCanvas(rt);

    rt = MakeSignCanvas();
    BeginTextureMode(rt);
        ClearBackground((Color){ 14, 22, 34, 255 });
        DrawRectangleLinesEx((Rectangle){ 4, 4, W - 8, H - 8 }, 3, (Color){ 110, 200, 235, 255 });
        DrawCircleLines(42, H / 2, 24, (Color){ 110, 200, 235, 255 });
        DrawCircleLines(42, H / 2, 16, (Color){ 110, 200, 235, 255 });
        DrawText("AIRLOCK", 84, H / 2 - 14, 28, (Color){ 225, 235, 245, 255 });
    signs[SIGN_AIRLOCK] = FinishSignCanvas(rt);

    signs[SIGN_DESIGNATION] = MakeDesignationSign("RK-8", "MAINTENANCE ACCESS");
    signs[SIGN_DESIGNATION_2] = MakeDesignationSign("DK-3", "DECK ACCESS");
    signs[SIGN_DESIGNATION_3] = MakeDesignationSign("EN-5", "ENGINEERING BAY");

    rt = MakeSignCanvas();
    BeginTextureMode(rt);
        ClearBackground((Color){ 40, 14, 14, 255 });
        DrawCircleLines(W / 2, 46, 30, (Color){ 235, 235, 235, 255 });
        DrawCircleLines(W / 2, 46, 29, (Color){ 235, 235, 235, 255 });
        DrawLineEx((Vector2){ W / 2.0f - 21, 25.0f }, (Vector2){ W / 2.0f + 21, 67.0f }, 4.0f, (Color){ 235, 235, 235, 255 });
        int nw = MeasureText("RESTRICTED", 20);
        DrawText("RESTRICTED", W / 2 - nw / 2, 88, 20, (Color){ 235, 200, 200, 255 });
    signs[SIGN_NO_ENTRY] = FinishSignCanvas(rt);

    rt = MakeSignCanvas();
    BeginTextureMode(rt);
        ClearBackground((Color){ 16, 18, 26, 255 });
        Vector2 c = { W / 2.0f, H / 2.0f };
        float r = 46.0f;
        DrawCircleLines((int)c.x, (int)c.y, r, (Color){ 140, 150, 170, 255 });
        DrawCircleLines((int)c.x, (int)c.y, r - 6, (Color){ 90, 100, 120, 255 });
        for (int i = 0; i < 12; i++) {
            float ang = i * (360.0f / 12.0f) * DEG2RAD;
            Vector2 a = { c.x + cosf(ang) * (r - 10), c.y + sinf(ang) * (r - 10) };
            Vector2 b = { c.x + cosf(ang) * r, c.y + sinf(ang) * r };
            DrawLineEx(a, b, 2.0f, (Color){ 170, 180, 200, 255 });
        }
        float needleAng = 220.0f * DEG2RAD;
        Vector2 needleEnd = { c.x + cosf(needleAng) * (r - 14), c.y + sinf(needleAng) * (r - 14) };
        DrawLineEx(c, needleEnd, 3.0f, (Color){ 235, 120, 90, 255 });
        DrawCircleV(c, 5.0f, (Color){ 235, 235, 235, 255 });
    signs[SIGN_GAUGE] = FinishSignCanvas(rt);

    rt = MakeSignCanvas();
    BeginTextureMode(rt);
        ClearBackground((Color){ 14, 20, 24, 255 });
        Color ln = (Color){ 90, 180, 170, 255 };
        DrawLine(30, 20, 30, H - 20, ln);
        DrawLine(30, 20, W - 40, 20, ln);
        DrawLine(W - 40, 20, W - 40, 60, ln);
        DrawLine(W - 40, 60, W - 80, 60, ln);
        DrawLine(W - 80, 60, W - 80, H - 20, ln);
        DrawLine(30, H - 20, W - 80, H - 20, ln);
        DrawLine(80, 20, 80, 50, ln);
        DrawCircle(30, 20, 4, (Color){ 110, 220, 200, 255 });
        DrawCircle(30, H - 20, 4, (Color){ 110, 220, 200, 255 });
        DrawCircle(W - 40, 20, 4, (Color){ 110, 220, 200, 255 });
        DrawCircle(W - 80, H - 20, 4, (Color){ 110, 220, 200, 255 });
        DrawRectangle(100, 70, 26, 18, (Color){ 235, 190, 90, 255 });
        DrawRectangle(150, 40, 18, 18, (Color){ 110, 220, 200, 255 });
    signs[SIGN_SCHEMATIC] = FinishSignCanvas(rt);

    rt = MakeSignCanvas();
    BeginTextureMode(rt);
        ClearBackground((Color){ 20, 22, 28, 255 });
        DrawRectangleLinesEx((Rectangle){ 10, 10, W - 20, H - 20 }, 3, (Color){ 110, 115, 128, 255 });
        for (int y = 22; y < H - 14; y += 14) {
            DrawRectangle(20, y, W - 40, 8, (Color){ 60, 64, 74, 255 });
        }
    signs[SIGN_VENT] = FinishSignCanvas(rt);

    rt = MakeSignCanvas();
    BeginTextureMode(rt);
        ClearBackground((Color){ 10, 10, 12, 255 });
        for (int i = -30; i < W + H; i += 26) {
            Vector2 p1 = { (float)i, (float)H };
            Vector2 p2 = { (float)(i + H), 0.0f };
            Vector2 p3 = { (float)(i + H - 14), 0.0f };
            Vector2 p4 = { (float)(i - 14), (float)H };
            DrawTriangle(p1, p2, p3, (Color){ 235, 190, 40, 255 });
            DrawTriangle(p1, p3, p4, (Color){ 235, 190, 40, 255 });
        }
    signs[SIGN_HAZARD_STRIPE] = FinishSignCanvas(rt);

    rt = MakeSignCanvas();
    BeginTextureMode(rt);
        ClearBackground((Color){ 18, 16, 26, 255 });
        DrawRectangleLinesEx((Rectangle){ 4, 4, W - 8, H - 8 }, 3, (Color){ 235, 200, 90, 255 });
        Color boltColor = (Color){ 245, 210, 90, 255 };
        Vector2 bolt[4] = {
            { W / 2.0f + 12.0f, 20.0f }, { W / 2.0f - 12.0f, 62.0f },
            { W / 2.0f + 4.0f, 62.0f }, { W / 2.0f - 18.0f, 108.0f }
        };
        DrawLineEx(bolt[0], bolt[1], 9.0f, boltColor);
        DrawLineEx(bolt[1], bolt[2], 9.0f, boltColor);
        DrawLineEx(bolt[2], bolt[3], 9.0f, boltColor);
        int pw = MeasureText("POWER", 22);
        DrawText("POWER", W - 60 - pw, H - 30, 22, (Color){ 225, 228, 235, 255 });
    signs[SIGN_POWER] = FinishSignCanvas(rt);

    rt = MakeSignCanvas();
    BeginTextureMode(rt);
        ClearBackground((Color){ 22, 24, 30, 255 });
        Vector2 vc = { W / 2.0f, H / 2.0f };
        DrawRing(vc, 30, 40, 0, 360, 24, (Color){ 150, 155, 165, 255 });
        for (int i = 0; i < 6; i++) {
            float ang = i * 60.0f * DEG2RAD;
            Vector2 a = { vc.x + cosf(ang) * 40, vc.y + sinf(ang) * 40 };
            Vector2 b = { vc.x + cosf(ang) * 54, vc.y + sinf(ang) * 54 };
            DrawLineEx(a, b, 5.0f, (Color){ 150, 155, 165, 255 });
        }
        DrawCircleV(vc, 14, (Color){ 90, 96, 108, 255 });
        DrawCircleLines((int)vc.x, (int)vc.y, 14, (Color){ 180, 185, 195, 255 });
    signs[SIGN_VALVE] = FinishSignCanvas(rt);
}

void Assets_LoadAll(void) {
    InitAudioDevice();

    textures.start = LoadTex("assets/images/start.png");
    textures.pause = LoadTex("assets/images/pause.png");
    textures.slidingtile = LoadTex("assets/images/slidingtile/slidingtile.png");
    SetTextureFilter(textures.start, TEXTURE_FILTER_BILINEAR);
    SetTextureFilter(textures.pause, TEXTURE_FILTER_BILINEAR);
    SetTextureFilter(textures.slidingtile, TEXTURE_FILTER_BILINEAR);

    audio.oot = LoadMusicStream(ResolvePath("assets/sounds/oot.mp3"));
    audio.oot.looping = true;

    audio.footsteps = LoadSnd("assets/sounds/playermaze/footsteps.wav");
    audio.door = LoadSnd("assets/sounds/playermaze/door.wav");
    audio.ambientEngine = LoadSnd("assets/sounds/playermaze/ambient_engine.wav");
    audio.minigameComplete = LoadSnd("assets/sounds/playermaze/minigamecomplete.wav");

    audio.colorConnected = LoadSnd("assets/sounds/flowfree/colorconnected.wav");
    audio.cardMatch = LoadSnd("assets/sounds/cardmatch/cardmatch.wav");
    audio.slidingMove = LoadSnd("assets/sounds/slidingtile/slidingtile.wav");
    audio.laser = LoadSnd("assets/sounds/spaceinvader/laser.wav");
    audio.explosion = LoadSnd("assets/sounds/spaceinvader/explosion.wav");
    audio.alienAttack = LoadSnd("assets/sounds/spaceinvader/alien_attack.wav");

    GenerateSigns();
}

void Assets_UnloadAll(void) {
    UnloadTexture(textures.start);
    UnloadTexture(textures.pause);
    UnloadTexture(textures.slidingtile);

    UnloadMusicStream(audio.oot);

    UnloadSound(audio.footsteps);
    UnloadSound(audio.door);
    UnloadSound(audio.ambientEngine);
    UnloadSound(audio.minigameComplete);
    UnloadSound(audio.colorConnected);
    UnloadSound(audio.cardMatch);
    UnloadSound(audio.slidingMove);
    UnloadSound(audio.laser);
    UnloadSound(audio.explosion);
    UnloadSound(audio.alienAttack);

    for (int i = 0; i < SIGN_COUNT; i++) UnloadTexture(signs[i]);

    CloseAudioDevice();
}

void Audio_Play(Sound sound) {
    if (app.settings.muted) return;
    PlaySound(sound);
}

void Audio_StopAllOneShots(void) {
    StopSound(audio.footsteps);
    StopSound(audio.door);
    StopSound(audio.ambientEngine);
    StopSound(audio.minigameComplete);
    StopSound(audio.colorConnected);
    StopSound(audio.cardMatch);
    StopSound(audio.slidingMove);
    StopSound(audio.laser);
    StopSound(audio.explosion);
    StopSound(audio.alienAttack);
}

void Audio_UpdateMusic(void) {
    UpdateMusicStream(audio.oot);
}

void Audio_SetMusicPlaying(bool shouldPlay) {
    bool isPlaying = IsMusicStreamPlaying(audio.oot);
    if (shouldPlay && !app.settings.muted) {
        if (!isPlaying) PlayMusicStream(audio.oot);
        SetMusicVolume(audio.oot, 1.0f);
    } else if (shouldPlay && app.settings.muted) {
        if (!isPlaying) PlayMusicStream(audio.oot);
        SetMusicVolume(audio.oot, 0.0f);
    } else {
        if (isPlaying) StopMusicStream(audio.oot);
    }
}
