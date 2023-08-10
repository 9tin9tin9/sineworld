#include <container/vec.h>
#include "graphics.h"
#include "input.h"
#include "movable.h"
#include "sprite.h"
#include "player.h"

static void Player_update(ecs_iter_t* it) {
    Position* position = ecs_field(it, Position, 2);
    Movable* movable = ecs_field(it, Movable, 3);
    const Graphics* graphics = ecs_singleton_get(it->world, Graphics);
    size_t w = graphics->w;
    size_t h = graphics->h;

    const Input* input = ecs_singleton_get(it->world, Input);
    for (Vec_iter(AI_Event) e = Vec_begin(input->events);
         e != Vec_end(input->events);
         e++) {
        if (e->type == AI_EVENTTYPE_MOUSE &&
            e->mouse.state == AI_MOUSE_DOWN) {
            movable->target.x = position->x + e->mouse.x - w/2.f;
            movable->target.y = position->y + e->mouse.y*2.f - h/2.f;
        }
    }
}

void Player_init(ecs_world_t* world) {
    ECS_TAG(world, Player);
    ecs_entity_t player = ecs_entity(world, { .name = "Player" });
    ecs_add(world, player, Player);
    ecs_set(world, player, Position, {0, 0});
    ecs_set(world, player, Movable,
            { .target = {0, 0}, .speed = 0.5 });
    ecs_set(world, player, Sprite,
            { .pixels =
                Vec_new(SpritePixel, { {0, 0}, AP_ColorRgb(255, 255, 0) },) });

    ecs_entity_t playerControlPhase = ecs_new_id(world);
    ecs_add_pair(world, playerControlPhase, EcsDependsOn, inputPhase);
    ecs_add_pair(world, EcsOnUpdate, EcsDependsOn, playerControlPhase);

    ECS_SYSTEM(world, Player_update, playerControlPhase,
               [none] Player, Position, Movable);
}
