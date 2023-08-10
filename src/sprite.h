#pragma once

#include <ansipixel/ansipixel.h>
#include <container/vec.h>
#include <flecs/flecs.h>
#include <raymath/raymath.h>

typedef struct {
    Vector2 pos;
    AP_ColorRgb color;
} SpritePixel;

typedef struct {
    Vec(SpritePixel) pixels;
} Sprite;
extern ECS_COMPONENT_DECLARE(Sprite);

extern ecs_entity_t spriteDrawPhase;
void Sprite_init(ecs_world_t* world);
