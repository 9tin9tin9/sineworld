#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <ansipixel/ansipixel.h>
#include <raymath/raymath.h>
#include "movable.h"
#include "sprite.h"
#include "terrain.h"
#include "graphics.h"

ECS_COMPONENT_DECLARE(Graphics);
ecs_entity_t terrainPhase;
ecs_entity_t drawPhase;

static void maxScreensize(size_t* height, size_t* width) {
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    *height = w.ws_row * 2;
    *width = w.ws_col;
}

static void Graphics_deinit(ecs_world_t *world, void *ctx) {
    AP_BufferRgb_del((struct AP_BufferRgb*)ctx);
    AP_resettextcolor();
    AP_clearScreen(NULL);
    AP_move(0, 0);
    AP_showcursor(true);
}

AP_ColorRgb drawMap(struct AP_BufferRgb* buf, size_t h, size_t w, Vector2 pos) {
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

    return Terrain_biomeMapColor(cf, cf, renderMin, renderMax, cb);
}

static void Graphics_update(ecs_iter_t* it) {
    Graphics* graphics = ecs_field(it, Graphics, 1);

    ecs_entity_t player = ecs_lookup(it->world, "Player");
    const Position* position = ecs_get(it->world, player, Position);
    AP_ColorRgb middle = drawMap(
        graphics->buf, graphics->h, graphics->w, *position);
    AP_ColorRgb playerColor = AP_ColorRgb(
        255 - AP_ColorRgb_r(middle),
        255 - AP_ColorRgb_g(middle),
        255 - AP_ColorRgb_b(middle));

    Sprite* s = ecs_get_mut(it->world, player, Sprite);
    s->pixels[0].color = playerColor;
}

static void Graphics_draw(ecs_iter_t* it) {
    Graphics* graphics = ecs_field(it, Graphics, 1);
    AP_BufferRgb_draw(graphics->buf);
}

void Graphics_init(ecs_world_t* world) {
    size_t h, w;
    maxScreensize(&h, &w);

    AP_clearScreen(NULL);
    AP_showcursor(false);

    struct AP_BufferRgb* buf = AP_BufferRgb_new(h, w);
    ECS_COMPONENT_DEFINE(world, Graphics);
    ecs_singleton_set(world, Graphics,
                      { .buf = buf, .h = h, .w = w });
    terrainPhase = ecs_new_id(world);
    ecs_add_pair(world, terrainPhase, EcsDependsOn, EcsOnStore);

    drawPhase = ecs_new_id(world);
    ecs_add_pair(world, drawPhase, EcsDependsOn, terrainPhase);

    ECS_SYSTEM(world, Graphics_update, terrainPhase, Graphics);
    ECS_SYSTEM(world, Graphics_draw, drawPhase, Graphics);
    ecs_atfini(world, Graphics_deinit, buf);
}
