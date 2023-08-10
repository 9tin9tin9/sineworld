#include <stdio.h>
#include <flecs/flecs.h>
#include "graphics.h"
#include "input.h"
#include "movable.h"
#include "player.h"
#include "sprite.h"
#include "ui.h"

void updateFPS(ecs_entity_t e, ecs_world_t* world, ecs_ftime_t dt) {
    float fps = 1/dt;
    int len = snprintf(NULL, 0, "%.2f", fps);
    char s[len + 1];
    snprintf(s, len + 1, "%.2f", fps);
    UI_Text* text = ecs_get_mut(world, e, UI_Text);
    Str_del(*text);
    ecs_set(world, e, UI_Text, { Str_from(s) });
}

void showFPS(ecs_world_t* world) {
    ecs_entity_t e = ecs_entity(world, { .name = "FPS Counter" });
    ecs_set(world, e, UI_Text, { Str_new() });
    ecs_set(world, e, Position, { 0, 0 });
    ecs_set(world, e, UI_Updatable, { updateFPS });
}

int main(int argc, char** argv) {
    ecs_world_t* world = ecs_init_w_args(argc, argv);

    Input_init(world);
    Graphics_init(world);
    Movable_init(world);
    Sprite_init(world);
    Player_init(world);
    UI_init(world);

    showFPS(world);

    ecs_set_target_fps(world, 24);
    while (ecs_progress(world, 0));

    return ecs_fini(world);
}
