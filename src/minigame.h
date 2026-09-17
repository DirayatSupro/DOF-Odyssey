#ifndef MINIGAME_H
#define MINIGAME_H

typedef enum {
    MG_RUNNING,
    MG_SUCCESS
} MinigameStatus;

typedef struct {
    void (*Init)(void);
    MinigameStatus (*Update)(float dt);
    void (*Draw)(void);
} Minigame;

Minigame Trivia_GetInterface(void);
Minigame FlowFree_GetInterface(void);
Minigame Sliding_GetInterface(void);
Minigame CardMatch_GetInterface(void);
Minigame SpaceInvader_GetInterface(void);

// Picks the right minigame for a 1..5 level index.
Minigame Minigame_ForLevel(int levelIndex);

#endif
