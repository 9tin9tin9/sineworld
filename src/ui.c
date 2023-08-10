#include "graphics.h"
#include "input.h"
#include "movable.h"
#include "sprite.h"
#include "ui.h"

ECS_COMPONENT_DECLARE(UI_Text);
ECS_COMPONENT_DECLARE(UI_Updatable);
ECS_COMPONENT_DECLARE(UI_Clickable);

void UI_Updatable_update(ecs_iter_t* it) {
    UI_Updatable* updatables = ecs_field(it, UI_Updatable, 1);
    ecs_entity_t* entities = it->entities;

    for (int i = 0; i < it->count; i++) {
        updatables[i](entities[i], it->world, it->delta_time);
    }
}

void UI_Clickable_update(ecs_iter_t* it) {
    UI_Clickable* clickables = ecs_field(it, UI_Clickable, 1);
    ecs_entity_t* entities = it->entities;
    const Input* input = ecs_singleton_get(it->world, Input);

    for (int i = 0; i < it->count; i++) {
        clickables[i](entities[i], input->events);
    }
}

void UI_Text_draw(ecs_iter_t* it) {
    UI_Text* texts = ecs_field(it, UI_Text, 1);
    Position* positions = ecs_field(it, Position, 2);
    const Graphics* graphics = ecs_singleton_get(it->world, Graphics);

    struct AP_BufferRgb* buf = graphics->buf;
    for (int i = 0; i < it->count; i++) {
        size_t y = positions->y;
        size_t x = positions->x;
        for (int c = 0; c < Str_len(texts[i]); c++) {
            AP_BufferRgb_setPixel(buf, y * 2, x + c, 0);
            AP_BufferRgb_setPixel(buf, y * 2 + 1, x + c, 0);
        }
        AP_move(y, x);
        AP_resettextcolor();
        fputs(texts[i], stdout);
        fflush(stdout);
    }
}

void UI_init(ecs_world_t* world) {
    ECS_COMPONENT_DEFINE(world, UI_Text);
    ECS_COMPONENT_DEFINE(world, UI_Updatable);
    ECS_COMPONENT_DEFINE(world, UI_Clickable);

    uiInputPhase = ecs_new_id(world);
    uiUpdatePhase = ecs_new_id(world);
    uiDrawPhase = ecs_new_id(world);

    ecs_add_pair(world, uiInputPhase, EcsDependsOn, inputPhase);
    ecs_add_pair(world, EcsOnUpdate, EcsDependsOn, uiInputPhase);

    ecs_add_pair(world, uiUpdatePhase, EcsDependsOn, EcsOnUpdate);
    ecs_add_pair(world, uiDrawPhase, EcsDependsOn, uiUpdatePhase);

    ecs_add_pair(world, uiDrawPhase, EcsDependsOn, spriteDrawPhase);
    ecs_add_pair(world, drawPhase, EcsDependsOn, uiDrawPhase);

    ECS_SYSTEM(world, UI_Updatable_update, uiUpdatePhase, UI_Updatable);
    ECS_SYSTEM(world, UI_Clickable_update, uiInputPhase, UI_Clickable);
    ECS_SYSTEM(world, UI_Text_draw, uiDrawPhase, UI_Text, Position);
}
