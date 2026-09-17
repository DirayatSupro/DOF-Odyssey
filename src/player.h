#ifndef PLAYER_H
#define PLAYER_H

#include "raylib.h"
#include "maze.h"
#include <stdbool.h>

typedef struct {
    Vector3 position;
    float yawDeg;
    bool isMoving;
} Player;

void Player_Reset(Player *p, Vector3 startPos, float startYawDeg);

// dofMask uses the DofFlag bits from app.h. Handles mouse-look + gated
// WASD/arrow movement with collision against the maze, and keeps the
// footsteps loop playing for exactly as long as the player is moving.
void Player_Update(Player *p, float dt, unsigned int dofMask, const Maze *maze, bool kioskSolved);

Camera3D Player_GetCamera(const Player *p);

#endif
