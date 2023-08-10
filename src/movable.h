#pragma once

#include <flecs/flecs.h>
#include <raymath/raymath.h>

typedef Vector2 Position;
extern ECS_COMPONENT_DECLARE(Position);

typedef struct {
    Vector2 target;
    float speed; // distance per (1/30) sec
} Movable;
extern ECS_COMPONENT_DECLARE(Movable);

void Movable_init(ecs_world_t* world);
