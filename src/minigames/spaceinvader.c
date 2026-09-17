#include "minigame.h"
#include "assets.h"
#include "theme.h"
#include "viewport.h"
#include <math.h>
#include <string.h>

#define SI_COLS 8
#define SI_ROWS 3
#define SI_ALIEN_COUNT (SI_COLS * SI_ROWS)
#define SI_MAX_PLAYER_BULLETS 8
#define SI_MAX_ENEMY_BULLETS 16

typedef struct {
    bool alive;
    int col, row;
} Alien;

typedef struct {
    bool active;
    Vector2 pos;
} Bullet;

static Alien aliens[SI_ALIEN_COUNT];
static int aliveCount;
static float alienOriginX, alienOriginY;
static float alienDirX;
static float alienSpeed;
static const float alienStepDown = 24.0f;
static const float alienW = 46.0f, alienH = 30.0f, alienGapX = 18.0f, alienGapY = 18.0f;

static Bullet playerBullets[SI_MAX_PLAYER_BULLETS];
static Bullet enemyBullets[SI_MAX_ENEMY_BULLETS];
static float fireCooldown;
static float enemyFireTimer;

static Rectangle playArea;
static float shipX;
static bool shipAtFront;
static float frontY, backY;
static const float shipW = 54.0f, shipH = 26.0f;

static Rectangle shieldRect;
static int shieldHp;
static const int shieldMaxHp = 4;

static int hearts;
static int score;
static float loseFlashTimer;

static void ResetRound(void) {
    for (int r = 0; r < SI_ROWS; r++) {
        for (int c = 0; c < SI_COLS; c++) {
            aliens[r * SI_COLS + c] = (Alien){ true, c, r };
        }
    }
    aliveCount = SI_ALIEN_COUNT;

    float blockW = SI_COLS * alienW + (SI_COLS - 1) * alienGapX;
    alienOriginX = playArea.x + playArea.width / 2.0f - blockW / 2.0f;
    alienOriginY = playArea.y + 30.0f;
    alienDirX = 1.0f;
    alienSpeed = 40.0f;

    for (int i = 0; i < SI_MAX_PLAYER_BULLETS; i++) playerBullets[i].active = false;
    for (int i = 0; i < SI_MAX_ENEMY_BULLETS; i++) enemyBullets[i].active = false;
    fireCooldown = 0.0f;
    enemyFireTimer = 0.6f;

    shipX = playArea.x + playArea.width / 2.0f;
    shipAtFront = true;

    shieldRect = (Rectangle){ playArea.x + playArea.width / 2.0f - 70, backY - 110, 140, 34 };
    shieldHp = shieldMaxHp;

    hearts = 3;
    score = 0;
    loseFlashTimer = 0.0f;
}

static void SpaceInvaderInit(void) {
    playArea = (Rectangle){ 110, 110, (float)VIRTUAL_WIDTH - 220, 480 };
    frontY = playArea.y + playArea.height - 120;
    backY = playArea.y + playArea.height - 40;
    ResetRound();
}

static Rectangle AlienRect(const Alien *a) {
    float x = alienOriginX + a->col * (alienW + alienGapX);
    float y = alienOriginY + a->row * (alienH + alienGapY);
    return (Rectangle){ x, y, alienW, alienH };
}

static Rectangle ShipRect(void) {
    float y = shipAtFront ? frontY : backY;
    return (Rectangle){ shipX - shipW / 2, y - shipH / 2, shipW, shipH };
}

static void SpawnPlayerBullet(void) {
    for (int i = 0; i < SI_MAX_PLAYER_BULLETS; i++) {
        if (!playerBullets[i].active) {
            playerBullets[i].active = true;
            Rectangle sr = ShipRect();
            playerBullets[i].pos = (Vector2){ sr.x + sr.width / 2, sr.y };
            Audio_Play(audio.laser);
            return;
        }
    }
}

static void SpawnEnemyBullet(Vector2 from) {
    for (int i = 0; i < SI_MAX_ENEMY_BULLETS; i++) {
        if (!enemyBullets[i].active) {
            enemyBullets[i].active = true;
            enemyBullets[i].pos = from;
            return;
        }
    }
}

static MinigameStatus SpaceInvaderUpdate(float dt) {
    if (loseFlashTimer > 0.0f) {
        loseFlashTimer -= dt;
        return MG_RUNNING;
    }

    // Ship movement (free strafing).
    float moveSpeed = 340.0f;
    if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT)) shipX -= moveSpeed * dt;
    if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) shipX += moveSpeed * dt;
    if (shipX < playArea.x + shipW / 2) shipX = playArea.x + shipW / 2;
    if (shipX > playArea.x + playArea.width - shipW / 2) shipX = playArea.x + playArea.width - shipW / 2;

    if (IsKeyPressed(KEY_W) || IsKeyPressed(KEY_UP)) shipAtFront = true;
    if (IsKeyPressed(KEY_S) || IsKeyPressed(KEY_DOWN)) shipAtFront = false;

    fireCooldown -= dt;
    if (IsKeyDown(KEY_SPACE) && fireCooldown <= 0.0f) {
        SpawnPlayerBullet();
        fireCooldown = 0.35f;
    }

    // Alien swarm movement.
    float blockW = SI_COLS * alienW + (SI_COLS - 1) * alienGapX;
    float leftBound = playArea.x;
    float rightBound = playArea.x + playArea.width - blockW;
    alienOriginX += alienDirX * alienSpeed * (1.0f + (float)(SI_ALIEN_COUNT - aliveCount) * 0.03f) * dt;
    if (alienOriginX < leftBound || alienOriginX > rightBound) {
        alienDirX *= -1.0f;
        alienOriginY += alienStepDown;
    }

    // Enemy firing.
    enemyFireTimer -= dt;
    if (enemyFireTimer <= 0.0f && aliveCount > 0) {
        int tries = 0;
        while (tries < 8) {
            int idx = GetRandomValue(0, SI_ALIEN_COUNT - 1);
            if (aliens[idx].alive) {
                Rectangle ar = AlienRect(&aliens[idx]);
                SpawnEnemyBullet((Vector2){ ar.x + ar.width / 2, ar.y + ar.height });
                Audio_Play(audio.alienAttack);
                break;
            }
            tries++;
        }
        enemyFireTimer = 0.9f - (float)(SI_ALIEN_COUNT - aliveCount) * 0.015f;
        if (enemyFireTimer < 0.25f) enemyFireTimer = 0.25f;
    }

    // Update player bullets.
    for (int i = 0; i < SI_MAX_PLAYER_BULLETS; i++) {
        if (!playerBullets[i].active) continue;
        playerBullets[i].pos.y -= 480.0f * dt;
        if (playerBullets[i].pos.y < playArea.y) { playerBullets[i].active = false; continue; }

        Rectangle bulletRect = { playerBullets[i].pos.x - 2, playerBullets[i].pos.y - 8, 4, 16 };
        for (int a = 0; a < SI_ALIEN_COUNT; a++) {
            if (!aliens[a].alive) continue;
            if (CheckCollisionRecs(bulletRect, AlienRect(&aliens[a]))) {
                aliens[a].alive = false;
                aliveCount--;
                score += 10;
                playerBullets[i].active = false;
                Audio_Play(audio.explosion);
                break;
            }
        }
    }

    // Update enemy bullets.
    for (int i = 0; i < SI_MAX_ENEMY_BULLETS; i++) {
        if (!enemyBullets[i].active) continue;
        enemyBullets[i].pos.y += 300.0f * dt;

        Rectangle bulletRect = { enemyBullets[i].pos.x - 2, enemyBullets[i].pos.y - 8, 4, 16 };

        if (shieldHp > 0 && CheckCollisionRecs(bulletRect, shieldRect)) {
            shieldHp--;
            enemyBullets[i].active = false;
            Audio_Play(audio.explosion);
            continue;
        }

        if (CheckCollisionRecs(bulletRect, ShipRect())) {
            enemyBullets[i].active = false;
            hearts--;
            Audio_Play(audio.explosion);
            if (hearts <= 0) {
                loseFlashTimer = 1.2f;
                ResetRound();
            }
            continue;
        }

        if (enemyBullets[i].pos.y > playArea.y + playArea.height + 20) enemyBullets[i].active = false;
    }

    // Aliens reaching the player's line = round lost, try again.
    float lowestAlienY = 0.0f;
    for (int a = 0; a < SI_ALIEN_COUNT; a++) {
        if (!aliens[a].alive) continue;
        Rectangle ar = AlienRect(&aliens[a]);
        if (ar.y + ar.height > lowestAlienY) lowestAlienY = ar.y + ar.height;
    }
    if (aliveCount > 0 && lowestAlienY >= frontY - 30.0f) {
        loseFlashTimer = 1.2f;
        ResetRound();
    }

    if (aliveCount == 0) return MG_SUCCESS;
    return MG_RUNNING;
}

static void SpaceInvaderDraw(void) {
    int sw = VIRTUAL_WIDTH;
    DrawCenteredText("STAR DEFENDER", sw / 2, 24, 30, COL_TEXT);
    DrawText(TextFormat("Enemies %d", aliveCount), (int)playArea.x, 64, 18, COL_TEXT_DIM);
    DrawText(TextFormat("Score %d", score), (int)(playArea.x + playArea.width - 120), 64, 18, COL_TEXT_DIM);

    for (int i = 0; i < hearts; i++) {
        DrawText("<3", (int)(playArea.x + i * 30), 90, 20, (Color){ 235, 100, 100, 255 });
    }

    DrawRectangleLinesEx(playArea, 2.0f, COL_PANEL_BORDER);
    DrawLine((int)playArea.x, (int)backY - 6, (int)(playArea.x + playArea.width), (int)backY - 6, (Color){ 90, 95, 130, 120 });
    DrawText("BACK", (int)playArea.x + 4, (int)backY - 22, 12, COL_TEXT_DIM);
    DrawLine((int)playArea.x, (int)frontY - 6, (int)(playArea.x + playArea.width), (int)frontY - 6, (Color){ 90, 95, 130, 120 });
    DrawText("FRONT", (int)playArea.x + 4, (int)frontY - 22, 12, COL_TEXT_DIM);

    const Color rowColors[SI_ROWS] = { (Color){ 235, 130, 180, 255 }, (Color){ 120, 200, 235, 255 }, (Color){ 235, 200, 100, 255 } };
    for (int a = 0; a < SI_ALIEN_COUNT; a++) {
        if (!aliens[a].alive) continue;
        Rectangle r = AlienRect(&aliens[a]);
        Color c = rowColors[aliens[a].row];
        DrawRectangleRounded(r, 0.3f, 4, c);
        DrawCircle((int)(r.x + r.width * 0.3f), (int)(r.y + r.height * 0.45f), 4, (Color){ 20, 20, 30, 255 });
        DrawCircle((int)(r.x + r.width * 0.7f), (int)(r.y + r.height * 0.45f), 4, (Color){ 20, 20, 30, 255 });
        DrawRectangle((int)(r.x + 4), (int)(r.y + r.height), 4, 6, c);
        DrawRectangle((int)(r.x + r.width - 8), (int)(r.y + r.height), 4, 6, c);
    }

    if (shieldHp > 0) {
        Color shieldColor = (Color){ 90, 200, 140, (unsigned char)(120 + shieldHp * 30) };
        DrawRectangleRounded(shieldRect, 0.4f, 6, shieldColor);
    }

    Rectangle ship = ShipRect();
    Vector2 top = { ship.x + ship.width / 2, ship.y };
    Vector2 bl = { ship.x, ship.y + ship.height * 0.6f };
    Vector2 br = { ship.x + ship.width, ship.y + ship.height * 0.6f };
    DrawTriangle(top, bl, br, (Color){ 120, 200, 245, 255 });
    DrawRectangle((int)ship.x, (int)(ship.y + ship.height * 0.55f), (int)ship.width, (int)(ship.height * 0.45f), (Color){ 120, 200, 245, 255 });

    for (int i = 0; i < SI_MAX_PLAYER_BULLETS; i++) {
        if (!playerBullets[i].active) continue;
        DrawRectangle((int)playerBullets[i].pos.x - 2, (int)playerBullets[i].pos.y - 8, 4, 16, (Color){ 120, 200, 245, 255 });
    }
    for (int i = 0; i < SI_MAX_ENEMY_BULLETS; i++) {
        if (!enemyBullets[i].active) continue;
        DrawRectangle((int)enemyBullets[i].pos.x - 2, (int)enemyBullets[i].pos.y - 8, 4, 16, (Color){ 235, 120, 90, 255 });
    }

    if (loseFlashTimer > 0.0f) {
        DrawRectangle((int)playArea.x, (int)playArea.y, (int)playArea.width, (int)playArea.height, Fade(RED, 0.25f));
        DrawCenteredText("Systems hit - regrouping...", sw / 2, (int)(playArea.y + playArea.height / 2), 24, RAYWHITE);
    }

    DrawText("A/D strafe  W/S front-back  SPACE shoot", (int)playArea.x, (int)(playArea.y + playArea.height + 16), 16, COL_TEXT_DIM);
}

Minigame SpaceInvader_GetInterface(void) {
    Minigame m;
    m.Init = SpaceInvaderInit;
    m.Update = SpaceInvaderUpdate;
    m.Draw = SpaceInvaderDraw;
    return m;
}
