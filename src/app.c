#include "app.h"
#include "respath.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

AppState app;

void App_Init(void) {
    memset(&app, 0, sizeof(app));
    app.screen = SCREEN_START_MENU;
    app.pauseReturnTarget = SCREEN_PLAYING;
    app.running = true;
    Settings_Load();
    Save_Load();
    Leaderboard_Load();
}

void App_Shutdown(void) {
    Settings_Save();
}

void Settings_Load(void) {
    app.settings.muted = false;
    const char *path = ResolvePath("saves/settings.cfg");
    FILE *f = fopen(path, "r");
    if (!f) return;

    char line[128];
    while (fgets(line, sizeof(line), f)) {
        int value = 0;
        if (sscanf(line, "muted=%d", &value) == 1) app.settings.muted = (value != 0);
    }
    fclose(f);
}

void Settings_Save(void) {
    const char *path = ResolveWritablePath("saves/settings.cfg");
    FILE *f = fopen(path, "w");
    if (!f) return;
    fprintf(f, "muted=%d\n", app.settings.muted ? 1 : 0);
    fclose(f);
}

void Settings_ToggleMute(void) {
    app.settings.muted = !app.settings.muted;
    Settings_Save();
}

void Save_Load(void) {
    app.save.hasSave = false;
    const char *path = ResolvePath("saves/save.dat");
    FILE *f = fopen(path, "r");
    if (!f) return;

    char line[256];
    SaveData data;
    memset(&data, 0, sizeof(data));
    bool sawLevel = false;
    while (fgets(line, sizeof(line), f)) {
        int ival = 0;
        float fval = 0.0f;
        char sval[MAX_PLAYER_NAME];

        if (sscanf(line, "level=%d", &ival) == 1) { data.level = ival; sawLevel = true; }
        else if (sscanf(line, "dof=%u", &data.dof) == 1) { /* stored */ }
        else if (sscanf(line, "key=%d", &ival) == 1) { data.hasCockpitKey = (ival != 0); }
        else if (sscanf(line, "elapsed=%f", &fval) == 1) { data.elapsedSeconds = fval; }
        else if (sscanf(line, "name=%31[^\n]", sval) == 1) { strncpy(data.playerName, sval, MAX_PLAYER_NAME - 1); }
    }
    fclose(f);

    if (sawLevel) {
        data.hasSave = true;
        app.save = data;
    }
}

void Save_Write(void) {
    const char *path = ResolveWritablePath("saves/save.dat");
    FILE *f = fopen(path, "w");
    if (!f) return;
    fprintf(f, "level=%d\n", app.save.level);
    fprintf(f, "dof=%u\n", app.save.dof);
    fprintf(f, "key=%d\n", app.save.hasCockpitKey ? 1 : 0);
    fprintf(f, "elapsed=%.3f\n", app.save.elapsedSeconds);
    fprintf(f, "name=%s\n", app.save.playerName);
    fclose(f);
    app.save.hasSave = true;
}

void Save_Delete(void) {
    const char *path = ResolveWritablePath("saves/save.dat");
    remove(path);
    app.save.hasSave = false;
    memset(&app.save, 0, sizeof(app.save));
}

void Leaderboard_Load(void) {
    app.leaderboardCount = 0;
    const char *path = ResolvePath("saves/leaderboard.dat");
    FILE *f = fopen(path, "r");
    if (!f) return;

    char line[128];
    while (fgets(line, sizeof(line), f) && app.leaderboardCount < LEADERBOARD_MAX) {
        char name[MAX_PLAYER_NAME];
        float seconds = 0.0f;
        char *comma = strchr(line, ',');
        if (!comma) continue;
        size_t nameLen = (size_t)(comma - line);
        if (nameLen >= MAX_PLAYER_NAME) nameLen = MAX_PLAYER_NAME - 1;
        memcpy(name, line, nameLen);
        name[nameLen] = '\0';
        if (sscanf(comma + 1, "%f", &seconds) != 1) continue;

        LeaderboardEntry *e = &app.leaderboard[app.leaderboardCount++];
        strncpy(e->name, name, MAX_PLAYER_NAME - 1);
        e->seconds = seconds;
    }
    fclose(f);
}

static int CompareEntries(const void *a, const void *b) {
    const LeaderboardEntry *ea = (const LeaderboardEntry *)a;
    const LeaderboardEntry *eb = (const LeaderboardEntry *)b;
    if (ea->seconds < eb->seconds) return -1;
    if (ea->seconds > eb->seconds) return 1;
    return 0;
}

void Leaderboard_AddEntry(const char *name, float seconds) {
    Leaderboard_Load();

    if (app.leaderboardCount < LEADERBOARD_MAX) {
        LeaderboardEntry *e = &app.leaderboard[app.leaderboardCount++];
        strncpy(e->name, name, MAX_PLAYER_NAME - 1);
        e->name[MAX_PLAYER_NAME - 1] = '\0';
        e->seconds = seconds;
    } else {
        qsort(app.leaderboard, (size_t)app.leaderboardCount, sizeof(LeaderboardEntry), CompareEntries);
        if (seconds < app.leaderboard[LEADERBOARD_MAX - 1].seconds) {
            LeaderboardEntry *e = &app.leaderboard[LEADERBOARD_MAX - 1];
            strncpy(e->name, name, MAX_PLAYER_NAME - 1);
            e->name[MAX_PLAYER_NAME - 1] = '\0';
            e->seconds = seconds;
        }
    }

    qsort(app.leaderboard, (size_t)app.leaderboardCount, sizeof(LeaderboardEntry), CompareEntries);

    const char *path = ResolveWritablePath("saves/leaderboard.dat");
    FILE *f = fopen(path, "w");
    if (!f) return;
    for (int i = 0; i < app.leaderboardCount; i++) {
        fprintf(f, "%s,%.3f\n", app.leaderboard[i].name, app.leaderboard[i].seconds);
    }
    fclose(f);
}
