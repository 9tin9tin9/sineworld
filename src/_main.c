#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <time.h>
#include <unistd.h>

#include <ansiinput/ansiinput.h>
#include <ansipixel/ansipixel.h>
#include <raymath/raymath.h>

#include "terrain.h"

#define FPS 30
FILE* logfp;

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

float Vector2LineAngleBetter(Vector2 start, Vector2 end) {
    if (start.y == end.y && start.x > end.x) {
        return PI;
    }
    return Vector2LineAngle(start, end);
}

void drawMap(struct AP_BufferRgb* buf, size_t h, size_t w, Vector2 pos) {
    float* heightmap = malloc(h*w*sizeof(*heightmap));
    TerrainBiome* biomemap = malloc(h*w*sizeof(*biomemap));
    float max = 0, min = 0;
    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            Vector2 p = (Vector2){ .x = x - w/2.f, .y = y - h/2.f };
            p = Vector2Add(pos, p);
            p = Vector2Scale(p, TERRAIN_SCALE);
            float f = Terrain_terrain(p);
            heightmap[y*w + x] = f;
            max = f > max ? f : max;
            min = f < min ? f : min;

            TerrainBiome b = Terrain_biome(p, f);
            biomemap[y*w + x] = b;
        }
    }

    float cf = heightmap[(h/2)*w + (w/2)];
    TerrainBiome cb = biomemap[(h/2)*w + (w/2)];
    float renderMax = cf + 5;
    float renderMin = cf - 5;

    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            float pf = heightmap[y*w + x];
            pf = Clamp(pf, renderMin, renderMax);
            TerrainBiome b = biomemap[y*w + x];
            AP_ColorRgb rgb = 
                cb == BIOME_SEA && b != BIOME_SEA ?
                    AP_ColorRgb(0, 0, 0) :
                    Terrain_biomeMapColor(pf, cf, renderMin, renderMax, b);
            AP_BufferRgb_setPixel(buf, y, x, rgb);
        }
    }

    free(heightmap);
    free(biomemap);
}

#define printText(buf, y, x, ...) \
    do { \
        AP_resettextcolor(); \
        int _len = snprintf(NULL, 0, __VA_ARGS__); \
        for (int i = 0; i < _len; i++) { \
            AP_BufferRgb_setPixel(buf, y * 2, x + i, 0); \
            AP_BufferRgb_setPixel(buf, y * 2 + 1, x + i, 0); \
        } \
        AP_move(y, x); \
        printf(__VA_ARGS__); \
        fflush(stdout); \
    } while (0)

void waitFrame(uint64_t deltatimeUs) {
    const uint64_t fpstime = 1e6 / FPS;
    sleepInUs(fpstime > deltatimeUs ? (1e6 / FPS) - deltatimeUs : 0);
}

int main() {
    logfp = fopen("log.txt", "w");
    AI_raw();
    AI_noecho();
    AI_mouse();
    AI_initEventQueue();
    AI_setExitKey((AI_KeyEvent){ .ctrl = true, .alt = false, .c = 'C'});

    size_t h, w;
    maxScreensize(&h, &w);

    AP_clearScreen(NULL);
    AP_showcursor(false);
    struct AP_BufferRgb* buf = AP_BufferRgb_new(h, w);
    AP_BufferRgb_draw(buf);

    Vector2 pos = { 0 };
    Vector2 target = { 0 };
    AI_Event e = { 0 };
    float deltaDist = 0.5;
    uint64_t startframe, endframe = nowInUs();
    uint64_t deltatime = 0;
    while (!AI_shouldExit()) {
        startframe = nowInUs();

        while (AI_pollEvent(&e)) {
            if (e.type == AI_EVENTTYPE_MOUSE &&
                e.mouse.state == AI_MOUSE_DOWN) {
                target.x = pos.x + e.mouse.x - w/2.f;
                target.y = pos.y + e.mouse.y*2.f - h/2.f;
            }
        }
        if (!Vector2AproxEquals(pos, target, deltaDist)) {
            float angle = Vector2LineAngleBetter(pos, target);
            Vector2 deltapos = { .x = deltaDist, .y = 0 };
            deltapos = Vector2Rotate(deltapos, angle);
            pos = Vector2Add(pos, deltapos);
        }

        drawMap(buf, h, w, pos);
        AP_BufferRgb_setPixel(buf, h/2, w/2, AP_ColorRgb(255, 255, 0));
        printText(buf, 0, 0, "fps: %.2f", 1.0e6 / (deltatime));
        AP_BufferRgb_draw(buf);

        endframe = nowInUs();
        deltatime = endframe - startframe;
        waitFrame(deltatime);
    }

    AP_BufferRgb_del(buf);
    AP_resettextcolor();
    AP_clearScreen(NULL);
    AP_move(0, 0);
    AP_showcursor(true);

    AI_nomouse();
    AI_echo();
    AI_cooked();

    fclose(logfp);
    return 0;
}
