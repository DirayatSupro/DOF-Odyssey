#ifndef APP_H
#define APP_H

#include "raylib.h"
#include <stdbool.h>

typedef enum {
    SCREEN_START_MENU,
    SCREEN_SETTINGS,
    SCREEN_LEADERBOARD,
    SCREEN_GAME_SCRIPT,
    SCREEN_PLAYING,
    SCREEN_PAUSED,
    SCREEN_PAUSE_RESTART_CONFIRM,
    SCREEN_PAUSE_SETTINGS,
    SCREEN_PAUSE_SCRIPT
} AppScreen;

typedef enum {
    DOF_FORWARD  = 1 << 0,
    DOF_BACKWARD = 1 << 1,
    DOF_LEFT     = 1 << 2,
    DOF_RIGHT    = 1 << 3,
    DOF_SPACE    = 1 << 4
} DofFlag;

#define MAX_PLAYER_NAME 32
#define LEADERBOARD_MAX 10

typedef struct {
    char name[MAX_PLAYER_NAME];
    float seconds;
} LeaderboardEntry;

typedef struct {
    bool muted;
} Settings;

typedef struct {
    bool hasSave;
    int level;                 // 1..5, level currently in progress
    unsigned int dof;
    char playerName[MAX_PLAYER_NAME];
    float elapsedSeconds;
    bool hasCockpitKey;
} SaveData;

typedef struct {
    AppScreen screen;
    AppScreen pauseReturnTarget;   // screen to restore on Resume (always SCREEN_PLAYING today)
    Settings settings;
    SaveData save;
    int leaderboardCount;
    LeaderboardEntry leaderboard[LEADERBOARD_MAX];
    bool running;
} AppState;

extern AppState app;

void App_Init(void);
void App_Shutdown(void);

void Settings_Load(void);
void Settings_Save(void);
void Settings_ToggleMute(void);

void Save_Load(void);
void Save_Write(void);
void Save_Delete(void);

void Leaderboard_Load(void);
void Leaderboard_AddEntry(const char *name, float seconds);

#endif
