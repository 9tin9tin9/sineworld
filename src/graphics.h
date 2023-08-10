#pragma once

#include <stddef.h>
#include <ansipixel/ansipixel.h>
#include <flecs/flecs.h>

typedef struct {
    struct AP_BufferRgb* buf;
    size_t h, w;
} Graphics;
extern ECS_COMPONENT_DECLARE(Graphics);

extern ecs_entity_t terrainPhase;
extern ecs_entity_t drawPhase;
void Graphics_init(ecs_world_t* world);
