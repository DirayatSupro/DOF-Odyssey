#include "minigame.h"
#include "level.h"
#include "theme.h"
#include "viewport.h"
#include <string.h>
#include <ctype.h>

#define INPUT_MAX 63

typedef enum { STEP_NAME, STEP_QUESTION, STEP_CORRECT } TriviaStep;

static TriviaStep step;
static char input[INPUT_MAX + 1];
static int inputLen;
static char errorMsg[64];
static float correctTimer;

static void TrimTrailing(char *s) {
    int len = (int)strlen(s);
    while (len > 0 && (s[len - 1] == ' ' || s[len - 1] == '\t')) { s[--len] = '\0'; }
}

static bool EqualsIgnoreCase(const char *a, const char *b) {
    while (*a && *b) {
        if (tolower((unsigned char)*a) != tolower((unsigned char)*b)) return false;
        a++; b++;
    }
    return *a == '\0' && *b == '\0';
}

static void TriviaInit(void) {
    step = STEP_NAME;
    inputLen = 0;
    input[0] = '\0';
    errorMsg[0] = '\0';
    correctTimer = 0.0f;
}

static void UpdateTextInput(void) {
    int key = GetCharPressed();
    while (key > 0) {
        if (key >= 32 && key <= 125 && inputLen < INPUT_MAX) {
            input[inputLen++] = (char)key;
            input[inputLen] = '\0';
        }
        key = GetCharPressed();
    }
    if (IsKeyPressed(KEY_BACKSPACE) && inputLen > 0) {
        inputLen--;
        input[inputLen] = '\0';
    }
}

static MinigameStatus TriviaUpdate(float dt) {
    if (step == STEP_CORRECT) {
        correctTimer -= dt;
        if (correctTimer <= 0.0f) return MG_SUCCESS;
        return MG_RUNNING;
    }

    UpdateTextInput();

    if (IsKeyPressed(KEY_ENTER)) {
        char trimmed[INPUT_MAX + 1];
        strncpy(trimmed, input, INPUT_MAX);
        trimmed[INPUT_MAX] = '\0';
        TrimTrailing(trimmed);

        if (step == STEP_NAME) {
            if (strlen(trimmed) == 0) {
                strcpy(errorMsg, "Please enter a name.");
            } else {
                strncpy(levelState.playerName, trimmed, sizeof(levelState.playerName) - 1);
                levelState.playerName[sizeof(levelState.playerName) - 1] = '\0';
                step = STEP_QUESTION;
                inputLen = 0;
                input[0] = '\0';
                errorMsg[0] = '\0';
            }
        } else if (step == STEP_QUESTION) {
            if (EqualsIgnoreCase(trimmed, "BUET Robotics Society")) {
                step = STEP_CORRECT;
                correctTimer = 1.0f;
                errorMsg[0] = '\0';
            } else {
                strcpy(errorMsg, "Incorrect. Try again.");
                inputLen = 0;
                input[0] = '\0';
            }
        }
    }

    return MG_RUNNING;
}

static void TriviaDraw(void) {
    int sw = VIRTUAL_WIDTH;
    int sh = VIRTUAL_HEIGHT;
    Rectangle panel = { sw / 2.0f - 340, sh / 2.0f - 160, 680, 320 };
    DrawRectangleRounded(panel, 0.06f, 8, COL_PANEL);
    DrawRectangleRoundedLinesEx(panel, 0.06f, 8, 2.0f, COL_PANEL_BORDER);

    DrawCenteredText("BRS TERMINAL", sw / 2, (int)panel.y + 24, 26, COL_TEXT);

    const char *question;
    if (step == STEP_NAME) question = "Enter Player Name:";
    else if (step == STEP_QUESTION) question = "What is the full form of BRS?";
    else question = "Access granted.";

    DrawCenteredText(question, sw / 2, (int)panel.y + 90, 20, COL_TEXT);

    if (step != STEP_CORRECT) {
        Rectangle box = { panel.x + 60, panel.y + 150, panel.width - 120, 48 };
        DrawRectangleRounded(box, 0.2f, 8, (Color){ 12, 15, 32, 255 });
        DrawRectangleRoundedLinesEx(box, 0.2f, 8, 1.5f, COL_ACCENT);
        DrawText(input, (int)box.x + 16, (int)box.y + 12, 22, COL_TEXT);

        if (((int)(GetTime() * 2)) % 2 == 0) {
            int caretX = (int)box.x + 16 + MeasureText(input, 22);
            DrawText("_", caretX, (int)box.y + 12, 22, COL_ACCENT);
        }

        DrawCenteredText("Press ENTER to submit", sw / 2, (int)panel.y + 220, 16, COL_TEXT_DIM);

        if (errorMsg[0] != '\0') {
            DrawCenteredText(errorMsg, sw / 2, (int)panel.y + 250, 18, COL_DANGER);
        }
    } else {
        DrawCenteredText("Degree of Freedom unlocked: Moving Backwards", sw / 2, (int)panel.y + 160, 18, COL_SUCCESS);
    }
}

Minigame Trivia_GetInterface(void) {
    Minigame m;
    m.Init = TriviaInit;
    m.Update = TriviaUpdate;
    m.Draw = TriviaDraw;
    return m;
}
