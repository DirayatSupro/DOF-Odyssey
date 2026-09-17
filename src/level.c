#include "level.h"
#include "app.h"
#include "assets.h"
#include "theme.h"
#include "viewport.h"
#include <string.h>

LevelState levelState;

static unsigned int GrantFlagForLevel(int lvl) {
    switch (lvl) {
        case 1: return DOF_BACKWARD;
        case 2: return DOF_LEFT;
        case 3: return DOF_RIGHT;
        case 4: return DOF_SPACE;
        default: return 0;
    }
}

static const char *ambientSubtitles[] = {
    "Gotta reach the cockpit...",
    "Come on, Rocky. Keep moving.",
    "One system at a time. Focus.",
    "The Odyssey won't fix herself.",
    "Every degree of freedom counts.",
    "Stay sharp. Not out of this yet.",
    "The cockpit's waiting for you.",
    "Just a little further.",
    "Don't stop now.",
    "Whatever locked this down, it's not stopping me.",
};
#define AMBIENT_SUBTITLE_COUNT (int)(sizeof(ambientSubtitles) / sizeof(ambientSubtitles[0]))

static void QueueNextSubtitle(void) {
    if (AMBIENT_SUBTITLE_COUNT <= 1) {
        levelState.subtitleIndex = 0;
    } else {
        int idx;
        do { idx = GetRandomValue(0, AMBIENT_SUBTITLE_COUNT - 1); } while (idx == levelState.subtitleIndex);
        levelState.subtitleIndex = idx;
    }
    levelState.subtitleShowTimer = 4.5f;
    levelState.subtitleCooldown = (float)GetRandomValue(18, 34);
}

static const char *DofUnlockLabel(int levelIndex) {
    switch (levelIndex) {
        case 1: return "Moving Backwards";
        case 2: return "Moving Left";
        case 3: return "Moving Right";
        case 4: return "Using the Spacebar";
        default: return "the Key to the Cockpit";
    }
}

const char *Level_GetName(int levelIndex) {
    switch (levelIndex) {
        case 1: return "The Corridor";
        case 2: return "The Engine Room Maze";
        case 3: return "The Cargo Bay Maze";
        case 4: return "The Command Deck Maze";
        case 5: return "The Cockpit Maze";
        default: return "Unknown Sector";
    }
}

static void SyncToAppSave(void) {
    app.save.level = levelState.levelIndex;
    app.save.dof = levelState.dof;
    app.save.hasCockpitKey = levelState.hasCockpitKey;
    app.save.elapsedSeconds = levelState.totalElapsed;
    strncpy(app.save.playerName, levelState.playerName, MAX_PLAYER_NAME - 1);
    app.save.playerName[MAX_PLAYER_NAME - 1] = '\0';
}

static void EnterLevel(int levelIndex, bool solved) {
    levelState.levelIndex = levelIndex;
    Maze_InitLevel(&levelState.maze, levelIndex);
    Vector3 startPos = Maze_CellToWorld(&levelState.maze, levelState.maze.startCol, levelState.maze.startRow);
    startPos.y = EYE_HEIGHT;
    Player_Reset(&levelState.player, startPos, levelState.maze.startYawDeg);
    levelState.kioskSolved = solved;
    levelState.inMinigame = false;
    levelState.ending = false;
    levelState.endingTimer = 0.0f;
    levelState.subtitleShowTimer = 0.0f;
    levelState.subtitleCooldown = (float)GetRandomValue(8, 16);
    levelState.subtitleIndex = -1;
}

void Level_StartNew(const char *playerName) {
    memset(&levelState, 0, sizeof(levelState));
    strncpy(levelState.playerName, playerName, sizeof(levelState.playerName) - 1);
    levelState.dof = DOF_FORWARD;
    levelState.hasCockpitKey = false;
    levelState.totalElapsed = 0.0f;
    EnterLevel(1, false);
    SyncToAppSave();
    Save_Write();
}

void Level_LoadFromSave(void) {
    memset(&levelState, 0, sizeof(levelState));
    strncpy(levelState.playerName, app.save.playerName, sizeof(levelState.playerName) - 1);
    levelState.dof = app.save.dof ? app.save.dof : DOF_FORWARD;
    levelState.hasCockpitKey = app.save.hasCockpitKey;
    levelState.totalElapsed = app.save.elapsedSeconds;
    int lvl = app.save.level >= 1 ? app.save.level : 1;
    bool solved = (GrantFlagForLevel(lvl) != 0) ? ((levelState.dof & GrantFlagForLevel(lvl)) != 0) : levelState.hasCockpitKey;
    EnterLevel(lvl, solved);
}

void Level_RestartCurrentLevel(void) {
    levelState.dof &= ~GrantFlagForLevel(levelState.levelIndex);
    if (levelState.levelIndex == 5) levelState.hasCockpitKey = false;
    EnterLevel(levelState.levelIndex, false);
}

void Level_RestartWholeGame(void) {
    char name[32];
    strncpy(name, levelState.playerName, sizeof(name) - 1);
    name[sizeof(name) - 1] = '\0';
    Save_Delete();
    Level_StartNew(name);
}

void Level_OnScreenActivated(void) {
    if (levelState.inMinigame) {
        EnableCursor();
    } else {
        DisableCursor();
    }
}

void Level_OnScreenDeactivated(void) {
    EnableCursor();
    StopSound(audio.footsteps);
}

static void AdvanceToNextLevel(void) {
    Audio_Play(audio.door);
    EnterLevel(levelState.levelIndex + 1, false);
    SyncToAppSave();
    Save_Write();
}

#define ENDING_FLOAT_DURATION 5.0f

static void StartEndingSequence(void) {
    levelState.ending = true;
    levelState.endingTimer = 0.0f;
    Audio_StopAllOneShots();
    Audio_Play(audio.door);
}

static void FinishGame(void) {
    Leaderboard_AddEntry(levelState.playerName, levelState.totalElapsed);
    Save_Delete();
    app.screen = SCREEN_START_MENU;
    Audio_StopAllOneShots();
    EnableCursor();
}

void Level_Update(float dt) {
    levelState.totalElapsed += dt;
    if (levelState.dofBannerTimer > 0.0f) levelState.dofBannerTimer -= dt;

    if (levelState.ending) {
        levelState.endingTimer += dt;
        levelState.player.position.y += 1.7f * dt;
        levelState.player.yawDeg += 14.0f * dt;
        if (levelState.endingTimer >= ENDING_FLOAT_DURATION) {
            FinishGame();
        }
        return;
    }

    if (levelState.inMinigame) {
        MinigameStatus status = levelState.activeMinigame.Update(dt);
        if (status == MG_SUCCESS) {
            unsigned int flag = GrantFlagForLevel(levelState.levelIndex);
            if (flag != 0) levelState.dof |= flag;
            else levelState.hasCockpitKey = true;

            levelState.kioskSolved = true;
            levelState.inMinigame = false;
            levelState.dofBannerTimer = 3.2f;
            DisableCursor();
            Audio_StopAllOneShots();
            Audio_Play(audio.minigameComplete);
            Audio_Play(audio.door);

            SyncToAppSave();
            Save_Write();
        }
        return;
    }

    if (!IsSoundPlaying(audio.ambientEngine)) Audio_Play(audio.ambientEngine);

    if (levelState.subtitleShowTimer > 0.0f) levelState.subtitleShowTimer -= dt;
    levelState.subtitleCooldown -= dt;
    if (levelState.subtitleCooldown <= 0.0f) QueueNextSubtitle();

    Player_Update(&levelState.player, dt, levelState.dof, &levelState.maze, levelState.kioskSolved);

    if (!levelState.kioskSolved && Maze_NearKiosk(&levelState.maze, levelState.player.position, 2.4f)) {
        if (IsKeyPressed(KEY_E)) {
            levelState.activeMinigame = Minigame_ForLevel(levelState.levelIndex);
            levelState.activeMinigame.Init();
            levelState.inMinigame = true;
            StopSound(audio.ambientEngine);
            StopSound(audio.footsteps);
            EnableCursor();
        }
    }

    if (levelState.kioskSolved && Maze_ReachedExit(&levelState.maze, levelState.player.position, 1.6f)) {
        if (levelState.levelIndex < 5) {
            AdvanceToNextLevel();
        } else {
            StartEndingSequence();
        }
    }
}

static void DrawHud(void) {
    int minutes = (int)levelState.totalElapsed / 60;
    int seconds = (int)levelState.totalElapsed % 60;
    DrawText(TextFormat("%02d:%02d", minutes, seconds), VIRTUAL_WIDTH - 110, 16, 24, RAYWHITE);
    DrawText(TextFormat("Sector %d/5 - %s", levelState.levelIndex, Level_GetName(levelState.levelIndex)), 16, 16, 20, RAYWHITE);

    const char *dofLabel[5] = { "FWD", "BACK", "LEFT", "RIGHT", "SPACE" };
    unsigned int flags[5] = { DOF_FORWARD, DOF_BACKWARD, DOF_LEFT, DOF_RIGHT, DOF_SPACE };
    for (int i = 0; i < 5; i++) {
        Color c = (levelState.dof & flags[i]) ? (Color){ 90, 200, 140, 255 } : (Color){ 90, 95, 115, 255 };
        DrawRectangle(16 + i * 66, 46, 58, 22, c);
        DrawText(dofLabel[i], 16 + i * 66 + 6, 50, 14, BLACK);
    }

    if (!levelState.kioskSolved && Maze_NearKiosk(&levelState.maze, levelState.player.position, 2.4f)) {
        DrawCenteredText("Press [E] to interact", VIRTUAL_WIDTH / 2, VIRTUAL_HEIGHT - 90, 24, RAYWHITE);
    }

    if (levelState.subtitleShowTimer > 0.0f && levelState.subtitleIndex >= 0) {
        float alpha = 1.0f;
        if (levelState.subtitleShowTimer > 4.0f) alpha = (4.5f - levelState.subtitleShowTimer) / 0.5f;
        else if (levelState.subtitleShowTimer < 0.6f) alpha = levelState.subtitleShowTimer / 0.6f;
        if (alpha < 0.0f) alpha = 0.0f;
        if (alpha > 1.0f) alpha = 1.0f;

        const char *line = ambientSubtitles[levelState.subtitleIndex];
        int fontSize = 20;
        int textW = MeasureText(line, fontSize);
        DrawRectangleRounded((Rectangle){ VIRTUAL_WIDTH / 2.0f - textW / 2.0f - 18, VIRTUAL_HEIGHT - 142, (float)textW + 36, 34 },
                              0.4f, 6, Fade((Color){ 10, 12, 26, 180 }, alpha));
        DrawCenteredText(line, VIRTUAL_WIDTH / 2, VIRTUAL_HEIGHT - 135, fontSize, Fade((Color){ 210, 215, 235, 255 }, alpha));
    }

    if (levelState.dofBannerTimer > 0.0f) {
        const float totalDuration = 3.2f;
        float elapsed = totalDuration - levelState.dofBannerTimer;
        float fadeIn = elapsed < 0.3f ? elapsed / 0.3f : 1.0f;
        float fadeOut = levelState.dofBannerTimer < 0.45f ? levelState.dofBannerTimer / 0.45f : 1.0f;
        float alpha = fadeIn < fadeOut ? fadeIn : fadeOut;
        float y = 28.0f + (1.0f - fadeIn) * -22.0f;

        const char *sub = DofUnlockLabel(levelState.levelIndex);
        int titleSize = 22, subSize = 18;
        int titleW = MeasureText("DEGREE OF FREEDOM UNLOCKED", titleSize);
        int subW = MeasureText(sub, subSize);
        float panelW = (float)(titleW > subW ? titleW : subW) + 60.0f;
        Rectangle panel = { VIRTUAL_WIDTH / 2.0f - panelW / 2.0f, y, panelW, 70.0f };

        DrawRectangleRounded(panel, 0.2f, 8, Fade((Color){ 18, 22, 46, 235 }, alpha));
        DrawRectangleRoundedLinesEx(panel, 0.2f, 8, 2.0f, Fade(COL_ACCENT, alpha));
        DrawCenteredText("DEGREE OF FREEDOM UNLOCKED", VIRTUAL_WIDTH / 2, (int)(panel.y + 10), titleSize, Fade(COL_ACCENT, alpha));
        DrawCenteredText(sub, VIRTUAL_WIDTH / 2, (int)(panel.y + 38), subSize, Fade(COL_TEXT, alpha));
    }
}

static void DrawEndingOverlay(void) {
    float t = levelState.endingTimer / ENDING_FLOAT_DURATION;
    if (t > 1.0f) t = 1.0f;

    float alpha = 1.0f;
    if (t < 0.12f) alpha = t / 0.12f;
    else if (t > 0.8f) alpha = (1.0f - t) / 0.2f;
    if (alpha < 0.0f) alpha = 0.0f;

    float y = VIRTUAL_HEIGHT * 0.6f - t * 260.0f;
    DrawCenteredText("Congrats, you have completed the Game!", VIRTUAL_WIDTH / 2, (int)y, 32, Fade(COL_SUCCESS, alpha));
}

void Level_Draw(void) {
    if (levelState.inMinigame) {
        DrawSpaceBackdrop(VIRTUAL_WIDTH, VIRTUAL_HEIGHT);
        levelState.activeMinigame.Draw();
        DrawText("[ESC] Pause", 16, VIRTUAL_HEIGHT - 30, 18, (Color){ 150, 155, 180, 255 });
        return;
    }

    Camera3D cam = Player_GetCamera(&levelState.player);
    ClearBackground((Color){ 6, 8, 20, 255 });
    BeginMode3D(cam);
    Maze_Draw(&levelState.maze, levelState.kioskSolved, cam);
    EndMode3D();

    if (levelState.ending) {
        DrawEndingOverlay();
    } else {
        DrawHud();
    }
}
