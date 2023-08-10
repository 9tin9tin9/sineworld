#pragma once

#include <ansiinput/ansiinput.h>
#include <container/vec.h>
#include <flecs/flecs.h>

typedef struct {
    Vec(AI_Event) events; // events in that frame
} Input;
extern ECS_COMPONENT_DECLARE(Input);

extern ecs_entity_t inputPhase;
void Input_init(ecs_world_t* world);
