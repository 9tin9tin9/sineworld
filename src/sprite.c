#include <stdio.h>
#include <ansipixel/ansipixel.h>
#include "graphics.h"
#include "movable.h"
#include "sprite.h"

ECS_COMPONENT_DECLARE(Sprite);
ecs_entity_t spriteDrawPhase;

static void Sprite_draw(ecs_iter_t* it) {
    Sprite* sprite = ecs_field(it, Sprite, 1);
    Position* position = ecs_field(it, Position, 2);

    ecs_entity_t player = ecs_lookup(it->world, "Player");
    const Position* cameraPosition = ecs_get(it->world, player, Position);
    Graphics* graphics = ecs_singleton_get_mut(it->world, Graphics);
    struct AP_BufferRgb* buf = graphics->buf;

    for (Vec_iter(SpritePixel) p = Vec_begin(sprite->pixels);
         p != Vec_end(sprite->pixels);
         p++) {
        size_t y = position->y + p->pos.y - cameraPosition->y + graphics->h / 2.f;
        size_t x = position->x + p->pos.x - cameraPosition->x + graphics->w / 2.f;
        AP_BufferRgb_setPixel(buf, y, x, p->color);
    }
}

static void Sprite_deinit(ecs_iter_t* it) {
    Sprite* s = ecs_field(it, Sprite, 1);
    Vec_del(s->pixels);
}

void Sprite_init(ecs_world_t* world) {
    ECS_COMPONENT_DEFINE(world, Sprite);

    spriteDrawPhase = ecs_new_id(world);
    ecs_add_pair(world, spriteDrawPhase, EcsDependsOn, terrainPhase);
    ecs_add_pair(world, drawPhase, EcsDependsOn, spriteDrawPhase);

    ECS_SYSTEM(world, Sprite_draw, spriteDrawPhase, Sprite, Position);
    ecs_set_hooks(world, Sprite, { .on_remove = Sprite_deinit });
}
