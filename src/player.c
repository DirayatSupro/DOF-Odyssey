#include "player.h"
#include "app.h"
#include "assets.h"
#include <math.h>

#define MOVE_SPEED 5.4f
#define TURN_SPEED_MOUSE 0.12f

void Player_Reset(Player *p, Vector3 startPos, float startYawDeg) {
    p->position = startPos;
    p->yawDeg = startYawDeg;
    p->isMoving = false;
}

void Player_Update(Player *p, float dt, unsigned int dofMask, const Maze *maze, bool kioskSolved) {
    Vector2 mouseDelta = GetMouseDelta();
    p->yawDeg += mouseDelta.x * TURN_SPEED_MOUSE;
    if (IsKeyDown(KEY_Q)) p->yawDeg -= 90.0f * dt;
    if (IsKeyDown(KEY_R)) p->yawDeg += 90.0f * dt;

    float yawRad = p->yawDeg * DEG2RAD;
    Vector3 forward = { sinf(yawRad), 0.0f, -cosf(yawRad) };
    Vector3 right = { cosf(yawRad), 0.0f, sinf(yawRad) };

    float moveX = 0.0f, moveZ = 0.0f;

    if ((IsKeyDown(KEY_W) || IsKeyDown(KEY_UP)) && (dofMask & DOF_FORWARD)) {
        moveX += forward.x; moveZ += forward.z;
    }
    if ((IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN)) && (dofMask & DOF_BACKWARD)) {
        moveX -= forward.x; moveZ -= forward.z;
    }
    if ((IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT)) && (dofMask & DOF_LEFT)) {
        moveX -= right.x; moveZ -= right.z;
    }
    if ((IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) && (dofMask & DOF_RIGHT)) {
        moveX += right.x; moveZ += right.z;
    }

    float len = sqrtf(moveX * moveX + moveZ * moveZ);
    p->isMoving = len > 0.001f;
    if (p->isMoving) {
        moveX /= len;
        moveZ /= len;

        float newX = p->position.x + moveX * MOVE_SPEED * dt;
        float newZ = p->position.z + moveZ * MOVE_SPEED * dt;

        if (Maze_CircleWalkable(maze, newX, p->position.z, PLAYER_RADIUS, kioskSolved)) {
            p->position.x = newX;
        }
        if (Maze_CircleWalkable(maze, p->position.x, newZ, PLAYER_RADIUS, kioskSolved)) {
            p->position.z = newZ;
        }
    }

    // footsteps.wav is a long continuous walking loop, not a single tap, so
    // it just needs to be kept alive while moving and cut the instant the
    // player stops - not restarted on a fixed timer (that was chopping it
    // off before it ever got audible, then letting the last restart ring on
    // once the player stood still to interact).
    if (p->isMoving) {
        if (!IsSoundPlaying(audio.footsteps)) Audio_Play(audio.footsteps);
    } else {
        StopSound(audio.footsteps);
    }
}

Camera3D Player_GetCamera(const Player *p) {
    Camera3D cam = { 0 };
    cam.position = (Vector3){ p->position.x, EYE_HEIGHT, p->position.z };
    float yawRad = p->yawDeg * DEG2RAD;
    Vector3 forward = { sinf(yawRad), 0.0f, -cosf(yawRad) };
    cam.target = (Vector3){ cam.position.x + forward.x, cam.position.y, cam.position.z + forward.z };
    cam.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    cam.fovy = 70.0f;
    cam.projection = CAMERA_PERSPECTIVE;
    return cam;
}
