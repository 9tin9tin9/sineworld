#pragma once

#include <stdbool.h>
#include <stdio.h>

// simple function keys
#define AI_KEY_UP (128 + 'A')
#define AI_KEY_DOWN (128 + 'B')
#define AI_KEY_RIGHT (128 + 'C')
#define AI_KEY_LEFT (128 + 'D')
#define AI_KEY_END (128 + 'F')
#define AI_KEY_HOME (128 + 'H')
#define AI_KEY_F1 (128 + 'P')
#define AI_KEY_F2 (128 + 'Q')
#define AI_KEY_F3 (128 + 'R')
#define AI_KEY_F4 (128 + 'S')

#define AI_KEY_SHIFT_UP (256 + 'A')
#define AI_KEY_SHIFT_DOWN (256 + 'B')
#define AI_KEY_SHIFT_RIGHT (256 + 'C')
#define AI_KEY_SHIFT_LEFT (256 + 'D')
#define AI_KEY_SHIFT_END (256 + 'F')
#define AI_KEY_SHIFT_HOME (256 + 'H')
#define AI_KEY_SHIFT_F1 (256 + 'P')
#define AI_KEY_SHIFT_F2 (256 + 'Q')
#define AI_KEY_SHIFT_F3 (256 + 'R')
#define AI_KEY_SHIFT_F4 (256 + 'S')

// complex function keys
#define AI_KEY_F5 (128 + 15)
#define AI_KEY_F6 (128 + 17)
#define AI_KEY_F7 (128 + 18)
#define AI_KEY_F8 (128 + 19)
#define AI_KEY_F9 (128 + 20)
#define AI_KEY_F10 (128 + 21)
#define AI_KEY_F11 (128 + 23)
#define AI_KEY_F12 (128 + 24)
#define AI_KEY_F13 (128 + 25)
#define AI_KEY_F14 (128 + 26)
#define AI_KEY_F15 (128 + 28)
#define AI_KEY_F16 (128 + 29)
#define AI_KEY_F17 (128 + 31)
#define AI_KEY_F18 (128 + 32)
#define AI_KEY_F19 (128 + 33)
#define AI_KEY_F20 (128 + 34)

#define AI_KEY_SHIFT_F5 (256 + 15)
#define AI_KEY_SHIFT_F6 (256 + 17)
#define AI_KEY_SHIFT_F7 (256 + 18)
#define AI_KEY_SHIFT_F8 (256 + 19)
#define AI_KEY_SHIFT_F9 (256 + 20)
#define AI_KEY_SHIFT_F10 (256 + 21)
#define AI_KEY_SHIFT_F11 (256 + 23)
#define AI_KEY_SHIFT_F12 (256 + 24)
#define AI_KEY_SHIFT_F13 (256 + 25)
#define AI_KEY_SHIFT_F14 (256 + 26)
#define AI_KEY_SHIFT_F15 (256 + 28)
#define AI_KEY_SHIFT_F16 (256 + 29)
#define AI_KEY_SHIFT_F17 (256 + 31)
#define AI_KEY_SHIFT_F18 (256 + 32)
#define AI_KEY_SHIFT_F19 (256 + 33)
#define AI_KEY_SHIFT_F20 (256 + 34)

typedef struct {
    bool alt;
    bool ctrl;
    int c;
} AI_KeyEvent;

typedef enum : char {
    AI_MOUSE_DOWN = 0,
    AI_MOUSE_UP,
    AI_MOUSE_MOVE = 0b00100000,
    AI_MOUSE_SCROLL_DOWN = 0b0100000,
    AI_MOUSE_SCROLL_UP = 0b0100001,
} AI_MouseState;

typedef enum : char {
    AI_MOUSE_LEFT = 0,
    AI_MOUSE_MIDDLE = 1,
    AI_MOUSE_RIGHT = 2,
    AI_MOUSE_NONE = 3,
} AI_MouseButton;

typedef enum : char {
    AI_MOUSE_SHIFT = 4,
    AI_MOUSE_ALT = 8,
    AI_MOUSE_CTRL = 16,
} AI_MouseModifier;

typedef struct {
    AI_MouseState state;
    AI_MouseButton button;
    bool shift;
    bool alt;
    bool ctrl;
    unsigned int x;
    unsigned int y;
} AI_MouseEvent;

typedef enum {
    AI_EVENTTYPE_NULL,
    AI_EVENTTYPE_KEY,
    AI_EVENTTYPE_MOUSE,
} AI_EventType;

typedef struct {
    AI_EventType type;

    union {
        AI_KeyEvent key;
        AI_MouseEvent mouse;
    };
} AI_Event;

void AI_echo();
void AI_noecho();

void AI_cbreak();
void AI_nocbreak();

void AI_raw();
void AI_cooked();

void AI_mouse(); // needs cbreak / raw
void AI_nomouse();

void AI_setSigintKey(AI_KeyEvent key); // should only be called if raw mode is enabled
void AI_setExitKey(AI_KeyEvent key);

bool AI_KeyEvent_equal(AI_KeyEvent* l, AI_KeyEvent* r);
void AI_KeyEvent_printInfo(AI_Event* e, FILE* f);

bool AI_MouseEvent_equal(AI_MouseEvent* l, AI_MouseEvent* r);
void AI_MouseEvent_printInfo(AI_Event* e, FILE* f);

bool AI_Event_equal(AI_Event* l, AI_Event* r);
void AI_Event_printInfo(AI_Event* e, FILE* f);

void AI_initEventQueue();
// will trigger setSigintKey and check exitKey
bool AI_pollEvent(AI_Event* event);
// will trigger setSigintKey and check exitKey
bool AI_waitEvent(AI_Event* event);
bool AI_shouldExit();

