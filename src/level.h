#ifndef LEVEL_H
#define LEVEL_H

#include "player.h"
#include "maze.h"
#include "minigame.h"
#include <stdbool.h>

typedef struct {
    int levelIndex;   // 1..5
    Player player;
    Maze maze;
    bool kioskSolved;
    bool inMinigame;
    Minigame activeMinigame;
    unsigned int dof;
    bool hasCockpitKey;
    char playerName[32];
    float totalElapsed;
    float dofBannerTimer; // >0 while the "Degree of Freedom unlocked" banner is showing
} LevelState;

extern LevelState levelState;

void Level_StartNew(const char *playerName);
void Level_LoadFromSave(void);
void Level_RestartCurrentLevel(void);
void Level_RestartWholeGame(void);

// Call whenever app.screen transitions into/out of SCREEN_PLAYING so the
// mouse cursor mode (locked for FPS look vs free for minigame/menu UI)
// stays correct.
void Level_OnScreenActivated(void);
void Level_OnScreenDeactivated(void);

void Level_Update(float dt);
void Level_Draw(void);

const char *Level_GetName(int levelIndex);

#endif
