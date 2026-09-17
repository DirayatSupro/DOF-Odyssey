#include "assets.h"
#include "app.h"
#include "respath.h"

GameTextures textures;
GameAudio audio;

static Texture2D LoadTex(const char *rel) {
    return LoadTexture(ResolvePath(rel));
}

static Sound LoadSnd(const char *rel) {
    return LoadSound(ResolvePath(rel));
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
