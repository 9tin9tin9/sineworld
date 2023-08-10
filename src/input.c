#include <ansiinput/ansiinput.h>
#include <container/vec.h>
#include <flecs/flecs.h>
#include "input.h"

ECS_COMPONENT_DECLARE(Input);
ecs_entity_t inputPhase;

static void Input_update(ecs_iter_t* it) {
    Input* input = ecs_field(it, Input, 1);

    Vec_clear(input->events);
    AI_Event e;
    while (AI_pollEvent(&e)) {
        Vec_push_back(input->events, e);
    }

    if (AI_shouldExit()) {
        ecs_quit(it->world);
    }
}

static void Input_deinit(ecs_world_t* world, void* ctx) {
    AI_nomouse();
    AI_echo();
    AI_cooked();
    Vec_del((Vec(AI_Event))ctx);
}

void Input_init(ecs_world_t* world) {
    AI_raw();
    AI_noecho();
    AI_mouse();
    AI_initEventQueue();
    AI_setExitKey((AI_KeyEvent){ .ctrl = true, .alt = false, .c = 'C'});

    Vec(AI_Event) e =  Vec_new(AI_Event);
    ECS_COMPONENT_DEFINE(world, Input);
    ecs_singleton_set(world, Input, { e });

    inputPhase = ecs_new_id(world);
    ecs_add_pair(world, inputPhase, EcsDependsOn, EcsPreUpdate);
    ecs_add_pair(world, EcsOnUpdate, EcsDependsOn, inputPhase);

    ECS_SYSTEM(world, Input_update, inputPhase, Input);
    ecs_atfini(world, Input_deinit, (void*)e);
}
