#pragma once

#include "foundation\foundation.h"
#include <entt/entt.hpp>

using registry_t = entt::registry;

struct state_t
{
    fd::window_t p1;
    registry_t* p2;
};

enum update_result
{
    keep_running,
    quit
};

struct rpg_game_t
{
    state_t(*get_state)();
    void (*start)();
    update_result(*run_once)();
    void (*exit)();
};