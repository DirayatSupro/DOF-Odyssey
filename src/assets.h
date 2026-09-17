#ifndef ASSETS_H
#define ASSETS_H

#include "raylib.h"

typedef struct {
    Texture2D start;
    Texture2D pause;
    Texture2D slidingtile;
} GameTextures;

// Small procedurally-drawn wall signage - text placards and technical
// diagrams mounted on the maze walls so they read as an actual built ship
// interior instead of plain colored blocks. Generated once at startup
// (see assets.c) rather than shipped as image files.
typedef enum {
    SIGN_CAUTION,
    SIGN_AIRLOCK,
    SIGN_DESIGNATION,
    SIGN_NO_ENTRY,
    SIGN_GAUGE,
    SIGN_SCHEMATIC,
    SIGN_VENT,
    SIGN_COUNT
} SignId;

extern RenderTexture2D signs[SIGN_COUNT];

typedef struct {
    Music oot;

    Sound footsteps;
    Sound door;
    Sound ambientEngine;
    Sound minigameComplete;

    Sound colorConnected;
    Sound cardMatch;
    Sound slidingMove;
    Sound laser;
    Sound explosion;
    Sound alienAttack;
} GameAudio;

extern GameTextures textures;
extern GameAudio audio;

void Assets_LoadAll(void);
void Assets_UnloadAll(void);

// Volume-aware helpers that respect app.settings.muted.
void Audio_Play(Sound sound);
void Audio_UpdateMusic(void);
void Audio_SetMusicPlaying(bool shouldPlay);

// Cuts off every one-shot sfx immediately (used when leaving a minigame or
// ending the run, so e.g. a laser/explosion sound can't ring on into the
// next screen).
void Audio_StopAllOneShots(void);

#endif
