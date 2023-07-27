#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <time.h>
#include <unistd.h>

#include <ansiinput/ansiinput.h>
#include <ansipixel/ansipixel.h>
#include <raymath/raymath.h>

#include "terrain.h"

void sleepInUs(uint64_t us) {
    struct timespec ts;
    ts.tv_sec = us / 1000000;
    ts.tv_nsec = us % 1000000 * 1000;

    while (nanosleep(&ts, &ts) == -1 && errno == EINTR);
}

uint64_t nowInUs() {
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    return (uint64_t)(now.tv_sec) * 1000000 + now.tv_nsec / 1000;
}

void fillscreen(
    struct AP_Buffer* buf,
    size_t h,
    size_t w,
    AP_Color color) {
    for (int i = 0; i < h; i++) {
        for (int j = 0; j < w; j++) {
            AP_Buffer_setPixel(buf, i, j, color);
        }
    }
    AP_Buffer_draw(buf);
}

void maxScreensize(size_t* height, size_t* width) {
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    *height = w.ws_row * 2;
    *width = w.ws_col;
}

int Vector2AproxEquals(Vector2 p, Vector2 q, float e) {
    return fabsf(p.x - q.x) < e && fabsf(p.y - q.y) < e;
}

int main() {
    FILE* f = fopen("log.txt", "w");
    AI_raw();
    AI_noecho();
    AI_mouse();
    AI_initEventQueue();
    AI_setExitKey((AI_KeyEvent){ .ctrl = true, .alt = false, .c = 'C'});

    size_t h, w;
    maxScreensize(&h, &w);

    AP_clearScreen(NULL);
    AP_showcursor(false);
    struct AP_Buffer* buf = AP_Buffer_new(h, w);
    AP_Buffer_draw(buf);

    Vector2 pos = { 0 };
    Vector2 target = { 0 };
    AI_Event e = { 0 };
    float deltaDist = 0.5;
    while (!AI_shouldExit()) {
        while (AI_pollEvent(&e)) {
            // if (e.type == AI_EVENTTYPE_MOUSE &&
            //     e.mouse.state == AI_MOUSE_DOWN) {
            //     target.x = e.mouse.x - w/2.f + pos.x;
            //     target.y = e.mouse.y/2.f - h/2.f + pos.y;
            // }
            if (e.type == AI_EVENTTYPE_KEY) {
                switch (e.key.c) {
                    case AI_KEY_UP: target = pos; target.y -= deltaDist; break;
                    case AI_KEY_DOWN: target = pos; target.y += deltaDist; break;
                    case AI_KEY_LEFT: target = pos; target.x -= deltaDist; break;
                    case AI_KEY_RIGHT: target = pos; target.x += deltaDist; break;
                    default: break;
                }
            }
        }
        if (!Vector2AproxEquals(pos, target, deltaDist)) {
            fprintf(f, "pos { x %.2f y %.2f } target { x %.2f y %.2f } ",
                    pos.x, pos.y,
                    target.x, target.y);

            float angle = Vector2Angle(pos, target);
            fprintf(f, "angle %.2f ", angle * RAD2DEG);

            Vector2 deltapos = { .x = deltaDist, .y = 0 };
            deltapos = Vector2Rotate(deltapos, angle);
            fprintf(f, "delta { x %.2f y %.2f } ", deltapos.x ,deltapos.y);

            pos = Vector2Add(pos, deltapos);

            fputc('\n', f);
        }

        float* values = malloc(h*w*sizeof(float));
        float max = 0, min = 0;
        for (int y = 0; y < h; y++) {
            for (int x = 0; x < w; x++) {
                Vector2 p = (Vector2){ .x = x - w/2.f, .y = y - h/2.f };
                p = Vector2Add(pos, p);
                p = Vector2Scale(p, 0.05);
                float f = terrain(p);
                values[y*w + x] = f;
                max = f > max ? f : max;
                min = f < min ? f : min;
            }
        }

        for (int y = 0; y < h; y++) {
            for (int x = 0; x < w; x++) {
                float pf = values[y*w + x];
                float cf = values[(h/2)*w + (w/2)];
                unsigned char c = Remap(pf, min, max, 0, 255);
                // unsigned char c = pf < cf ?
                //     Remap(pf, min, cf, 0, 127) :
                //     Remap(pf, cf, max, 128, 255);
                AP_Color color = AP_rgbTo256(AP_ColorRgb(c, c, c));
                AP_Buffer_setPixel(buf, y, x, color);
            }
        }
        AP_Buffer_setPixel(buf, h/2, w/2, 3);

        AP_Buffer_draw(buf);
        free(values);
    }

    AP_Buffer_del(buf);
    AP_resettextcolor();
    AP_clearScreen(NULL);
    AP_move(0, 0);
    AP_showcursor(true);

    AI_nomouse();
    AI_echo();
    AI_cooked();

    return 0;
}
