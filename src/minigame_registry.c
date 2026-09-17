#include "minigame.h"

Minigame Minigame_ForLevel(int levelIndex) {
    switch (levelIndex) {
        case 1: return Trivia_GetInterface();
        case 2: return FlowFree_GetInterface();
        case 3: return Sliding_GetInterface();
        case 4: return CardMatch_GetInterface();
        case 5: return SpaceInvader_GetInterface();
        default: return Trivia_GetInterface();
    }
}
