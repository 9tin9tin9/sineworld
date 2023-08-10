#include "movable.h"

ECS_COMPONENT_DECLARE(Position);
ECS_COMPONENT_DECLARE(Movable);

static int Vector2AproxEquals(Vector2 p, Vector2 q, float e) {
    return fabsf(p.x - q.x) < e && fabsf(p.y - q.y) < e;
}

static float Vector2LineAngleBetter(Vector2 start, Vector2 end) {
    if (start.y == end.y && start.x > end.x) {
        return PI;
    }
    return Vector2LineAngle(start, end);
}

static void Movable_update(ecs_iter_t* it) {
    Position* p = ecs_field(it, Position, 1);
    Movable* m = ecs_field(it, Movable, 2);

    const ecs_world_info_t* info = ecs_get_world_info(it->world);
    for (int i = 0; i < it->count; i++) {
        // speed * dt / (1 / 30)
        float deltaDist = m[i].speed * it->delta_time * 30;

        if (!Vector2AproxEquals(p[i], m[i].target, deltaDist)) {
            float angle = Vector2LineAngleBetter(p[i], m[i].target);
            Vector2 deltapos = { .x = deltaDist, .y = 0 };
            deltapos = Vector2Rotate(deltapos, angle);
            p[i] = Vector2Add(p[i], deltapos);
        }
    }
}

void Movable_init(ecs_world_t* world) {
    ECS_COMPONENT_DEFINE(world, Position);
    ECS_COMPONENT_DEFINE(world, Movable);
    ECS_SYSTEM(world, Movable_update, EcsOnUpdate, Position, Movable);
}
