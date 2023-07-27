#include <fcntl.h>
#include <pthread.h>
#include <stdatomic.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>

#include "ansiinput.h"

#define CSI "\e["
#define CSI_L (sizeof(CSI) - 1)

typedef struct {
    AI_Event* data;
    size_t head;
    size_t tail; // index after last element. tail should never == head
    size_t capacity;
} AI_EventQueue;
static AI_EventQueue queue;
static pthread_mutex_t eventQueueMutex;

static AI_Event sigintKey = { .type = AI_EVENTTYPE_NULL };
static AI_Event exitKey = { .type = AI_EVENTTYPE_NULL };
static atomic_bool shouldExit = false;

void AI_echo() {
    struct termios term;
    tcgetattr(STDIN_FILENO, &term);
    term.c_lflag = term.c_lflag | ECHO;
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &term);
}

void AI_noecho() {
    struct termios term;
    tcgetattr(STDIN_FILENO, &term);
    term.c_lflag = term.c_lflag & ~ECHO;
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &term);
}

void AI_nocbreak() {
    struct termios term;
    tcgetattr(STDIN_FILENO, &term);
    term.c_lflag = term.c_lflag | ICANON;
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &term);
}

void AI_cbreak() {
    struct termios term;
    tcgetattr(STDIN_FILENO, &term);
    term.c_lflag = term.c_lflag & ~ICANON;
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &term);
}

void AI_raw() {
    struct termios term;
    tcgetattr(STDIN_FILENO, &term);
    term.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    term.c_oflag &= ~(OPOST);
    term.c_cflag |= (CS8);
    term.c_lflag &= ~(ICANON | IEXTEN | ISIG);
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &term);
}

void AI_cooked() {
    struct termios term;
    tcgetattr(STDIN_FILENO, &term);
    term.c_iflag |= BRKINT | ICRNL | INPCK | ISTRIP | IXON;
    term.c_oflag |= OPOST;
    term.c_cflag &= ~CS8;
    term.c_lflag |= ICANON | IEXTEN | ISIG;
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &term);
}

static void AI_wait() {
    int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
    if (flags == -1) {
        return;
    }
    fcntl(STDIN_FILENO, F_SETFL, flags ^ O_NONBLOCK);
}

static void AI_nowait() {
    int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
    if (flags == -1) {
        return;
    }
    fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK);
}

void AI_mouse() {
    fprintf(stderr, "\e[?1003h\e[?1006h");
}

void AI_nomouse() {
    fprintf(stderr, "\e[?1003l\e[?1006l");
}

void AI_setSigintKey(AI_KeyEvent key) {
    sigintKey.type = AI_EVENTTYPE_KEY;
    sigintKey.key = key;
}

void AI_setExitKey(AI_KeyEvent key) {
    exitKey.type = AI_EVENTTYPE_KEY;
    exitKey.key = key;
}

bool AI_KeyEvent_equal(AI_KeyEvent* l, AI_KeyEvent* r) {
    return
        l->alt == r->alt &&
        l->ctrl == r->ctrl &&
        l->c == r->c;
}

void AI_KeyEvent_printInfo(AI_Event* e, FILE* f) {
    fprintf(f, "KEY %d ALT %s CTRL %s",
        e->key.c,
        e->key.alt ? "ON" : "OFF",
        e->key.ctrl ? "ON" : "OFF");
}

bool AI_MouseEvent_equal(AI_MouseEvent* l, AI_MouseEvent* r) {
    return
        l->state == r->state &&
        l->button == r->button &&
        l->shift == r->shift &&
        l->alt == r->alt &&
        l->ctrl == r->ctrl &&
        l->x == r->x &&
        l->y == r->y;
}

void AI_MouseEvent_printInfo(AI_Event* e, FILE* f) {
    char* state = 
        e->mouse.state == AI_MOUSE_UP ? "UP" :
        e->mouse.state == AI_MOUSE_DOWN ? "DOWN" :
        e->mouse.state == AI_MOUSE_MOVE ? "MOVE" :
        e->mouse.state == AI_MOUSE_SCROLL_DOWN ? "SCROLL_DOWN" :
        e->mouse.state == AI_MOUSE_SCROLL_UP ? "SCROLL_UP" :
        "UNKNOWN";
    fprintf(f, "MOUSE %s BUTTON %d X %d Y %d SHIFT %s ALT %s CTRL %s",
        state,
        e->mouse.button,
        e->mouse.x,
        e->mouse.y,
        e->mouse.shift ? "ON" : "OFF",
        e->mouse.alt ? "ON" : "OFF",
        e->mouse.ctrl ? "ON" : "OFF");
}

bool AI_Event_equal(AI_Event* l, AI_Event* r) {
    if (l->type == r->type) {
        switch (l->type) {
            case AI_EVENTTYPE_KEY:
                return AI_KeyEvent_equal(&l->key, &r->key);
            case AI_EVENTTYPE_MOUSE:
                return AI_MouseEvent_equal(&l->mouse, &r->mouse);
            case AI_EVENTTYPE_NULL:
                return true;
        }
    }
    return false;
}

void AI_Event_printInfo(AI_Event* e, FILE* f) {
    switch (e->type) {
        case AI_EVENTTYPE_KEY: AI_KeyEvent_printInfo(e, f); break;
        case AI_EVENTTYPE_MOUSE: AI_MouseEvent_printInfo(e, f); break;
        case AI_EVENTTYPE_NULL: fprintf(f, "NULL EVENT"); break;
    }
}

void AI_initEventQueue() {
    pthread_mutex_init(&eventQueueMutex, NULL);
    queue.capacity = 16;
    queue.head = 0;
    queue.tail = 0;
    queue.data = malloc(queue.capacity * sizeof(*queue.data));
}

static bool isStdinEmpty() {
    int ch = getchar();
    if (ch == EOF) {
        return true;
    }
    ungetc(ch, stdin);
    return false;
}

static void readInput(char** buf, size_t* len, bool wait) {
    size_t cap = 256;
    *len = 0;
    *buf = malloc(cap);

    if (!wait) {
        AI_nowait();
    }

    int readsize = 0;
    while ((readsize = read(STDIN_FILENO, *buf + *len, cap)) > 0) {
        *len += readsize;
        if (*len + 1 == cap) {
            cap *= 2;
            *buf = realloc(*buf, cap);
        }
        if (isStdinEmpty()) {
            break;
        }
    }
    (*buf)[*len] = 0;

    if (!wait) {
        AI_wait();
    }
}

static AI_Event* eventQueue_begin() {
    return queue.data + queue.head;
}

static AI_Event* eventQueue_end() {
    return queue.data + queue.tail;
}

static AI_Event* eventQueue_next(AI_Event* event) {
    size_t index = event - queue.data;
    if (index == queue.capacity - 1) {
        return queue.data;
    }
    return queue.data + index + 1;
}

static void eventQueue_resize() {
    size_t newcap = queue.capacity * 2;
    AI_Event* newdata = malloc(newcap * sizeof(*newdata));
    size_t i = 0;
    for (AI_Event* event = eventQueue_begin();
         event != eventQueue_end();
         event = eventQueue_next(event), i++) {
        newdata[i] = *event;
    }

    free(queue.data);
    queue.data = newdata;
    queue.head = 0;
    queue.tail = queue.capacity;
    queue.capacity = newcap;
}

static size_t eventQueue_size() {
    return queue.tail +
        queue.capacity * (queue.head > queue.tail) -
        queue.head;
}

static void eventQueue_pushBack(AI_Event* event) {
    queue.data[queue.tail] = *event;
    queue.tail = queue.tail + 1 == queue.capacity ?
        0 :
        queue.tail + 1;
    if (queue.tail == queue.head) {
        eventQueue_resize();
    }
}

static AI_Event eventQueue_popFront() {
    if (!eventQueue_size()) {
        return (AI_Event) { .type = AI_EVENTTYPE_NULL };
    }
    AI_Event e = queue.data[queue.head];
    queue.head += 1;
    if (queue.head == queue.capacity) {
        queue.head = 0;
    }
    return e;
}

static AI_Event eventQueue_peekFront() {
    if (!eventQueue_size()) {
        return (AI_Event) { .type = AI_EVENTTYPE_NULL };
    }
    return queue.data[queue.head];
}

static int size_t_digits (size_t n) {
    if (n < 10) return 1;
    if (n < 100) return 2;
    if (n < 1000) return 3;
    if (n < 10000) return 4;
    if (n < 100000) return 5;
    if (n < 1000000) return 6;
    if (n < 10000000) return 7;
    if (n < 100000000) return 8;
    if (n < 1000000000) return 9;
    if (n < 10000000000) return 10;
    if (n < 100000000000) return 11;
    if (n < 1000000000000) return 12;
    if (n < 10000000000000) return 13;
    if (n < 100000000000000) return 14;
    if (n < 1000000000000000) return 15;
    if (n < 10000000000000000) return 16;
    if (n < 100000000000000000) return 17;
    if (n < 1000000000000000000) return 18;
    if (n < 10000000000000000000U) return 19;
    return 20;
}

static void parseInput(char* buf, size_t len) {
    if (!len) {
        return;
    }

    char* end = buf + len;
    // test complex sequence first
    while (buf < end) {
        size_t type;
        size_t x;
        size_t y;
        char c;
        char mod;
        char controlChar;
        // mouse events
        if (sscanf(buf, CSI "<%zu;%zu;%zu%c", &type, &x, &y, &c) == 4) {
            AI_Event event = {
                .type = AI_EVENTTYPE_MOUSE,
                .mouse = {
                    .state = c == 'M' ?
                        type & AI_MOUSE_MOVE ? AI_MOUSE_MOVE :
                            type & AI_MOUSE_SCROLL_UP ? AI_MOUSE_SCROLL_UP :
                            type & AI_MOUSE_SCROLL_DOWN ? AI_MOUSE_SCROLL_DOWN :
                            AI_MOUSE_DOWN :
                        AI_MOUSE_UP,
                    .button = type & AI_MOUSE_NONE,
                    .shift = type & AI_MOUSE_SHIFT,
                    .alt = type & AI_MOUSE_ALT,
                    .ctrl = type & AI_MOUSE_CTRL,
                    .x = x - 1,
                    .y = y - 1,
                },
            };
            eventQueue_pushBack(&event);
            buf += CSI_L +
                4 +
                size_t_digits(type) +
                size_t_digits(x) +
                size_t_digits(y);
            continue;
        }
        // complex function keys with modifiers
        if (sscanf(buf, CSI "%hhu;%c~", &c, &mod) == 2) {
            AI_Event event = {
                .type = AI_EVENTTYPE_KEY,
                .key = {
                    .alt = mod == 3 || mod == 4 || mod == 7 || mod == 8,
                    .ctrl = 5 <= mod && mod <= 8,
                    .c = (128 << !(mod % 2)) + c,
                }
            };
            eventQueue_pushBack(&event);
            buf += CSI_L + size_t_digits(c) + 3;
            continue;
        }
        // complex function keys
        if (sscanf(buf, CSI "%hhu~", &c) == 1) {
            AI_Event event = {
                .type = AI_EVENTTYPE_KEY,
                .key = {
                    .alt = false,
                    .ctrl = false,
                    .c = 128 + c,
                }
            };
            eventQueue_pushBack(&event);
            buf += CSI_L + size_t_digits(c) + 1;
            continue;
        }
        // simple function keys with modifiers
        if (sscanf(buf, CSI "1;%c%c", &mod, &c) == 2) {
            AI_Event event = {
                .type = AI_EVENTTYPE_KEY,
                .key = {
                    .alt = mod == 3 || mod == 4 || mod == 7 || mod == 8,
                    .ctrl = 5 <= mod && mod <= 8,
                    .c = (128 << !(mod % 2)) + c,
                }
            };
            eventQueue_pushBack(&event);
            buf += CSI_L + 4;
            continue;
        }
        // simple function keys
        if (sscanf(buf, "\e%c%c", &controlChar, &c) == 2) {
            AI_Event event = {
                .type = AI_EVENTTYPE_KEY,
                .key = {
                    .alt = false,
                    .ctrl = false,
                    .c = 128 + c,
                }
            };
            eventQueue_pushBack(&event);
            buf += 3;
            continue;
        }
        // meta - key
        if (sscanf(buf, "\e%c", &c) == 1) {
            AI_Event event = {
                .type = AI_EVENTTYPE_KEY,
                .key = {
                    .alt = true,
                    .ctrl = c < 27,
                    .c = c < 27 ? c + 64 : c,
                }
            };
            eventQueue_pushBack(&event);
            buf += 2;
            continue;
        }
        AI_Event event = {
            .type = AI_EVENTTYPE_KEY,
            .key = {
                .ctrl = *buf < 27,
                .alt = false,
                .c = *buf < 27 ? *buf + 64 : *buf,
            }
        };
        buf += 1;
        eventQueue_pushBack(&event);
    }
}

static void AI_pumpEvents(bool wait) {
    char* buf;
    size_t len;
    readInput(&buf, &len, wait);
    parseInput(buf, len);
    free(buf);
}

bool AI_pollEvent(AI_Event* e) {
    pthread_mutex_lock(&eventQueueMutex);

    if (!eventQueue_size()) {
        AI_pumpEvents(false);
    }

    *e = eventQueue_popFront();
    if (e->type != AI_EVENTTYPE_NULL && AI_Event_equal(e, &sigintKey)) {
        raise(2);
    }
    if (e->type != AI_EVENTTYPE_NULL && AI_Event_equal(e, &exitKey)) {
        shouldExit = true;
    }

    pthread_mutex_unlock(&eventQueueMutex);
    return e->type != AI_EVENTTYPE_NULL;
}

bool AI_waitEvent(AI_Event* e) {
    pthread_mutex_lock(&eventQueueMutex);

    if (!eventQueue_size()) {
        AI_pumpEvents(true);
    }

    *e = eventQueue_popFront();
    if (e->type != AI_EVENTTYPE_NULL && AI_Event_equal(e, &sigintKey)) {
        raise(2);
    }
    if (e->type != AI_EVENTTYPE_NULL && AI_Event_equal(e, &exitKey)) {
        shouldExit = true;
    }

    pthread_mutex_unlock(&eventQueueMutex);
    return true;
}

bool AI_shouldExit() {
    return shouldExit;
}
