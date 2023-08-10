#pragma once

#include <ansiinput/ansiinput.h>
#include <container/str.h>
#include <container/vec.h>
#include <flecs/flecs.h>
#include <raymath/raymath.h>

typedef Str UI_Text;
extern ECS_COMPONENT_DECLARE(UI_Text);

typedef void (*UI_Updatable)(ecs_entity_t, ecs_world_t*, ecs_ftime_t);
extern ECS_COMPONENT_DECLARE(UI_Updatable);

typedef void (*UI_Clickable)(ecs_entity_t, const Vec(AI_Event));
extern ECS_COMPONENT_DECLARE(UI_Clickable);

ecs_entity_t uiInputPhase;
ecs_entity_t uiUpdatePhase;
ecs_entity_t uiDrawPhase;
void UI_init(ecs_world_t* world);
