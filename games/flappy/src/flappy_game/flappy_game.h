#pragma once

struct flappy_game_t
{
    void (*start)();
    bool (*run_once)();
    void (*exit)();
};