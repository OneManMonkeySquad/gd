#pragma once

#include "api.h"
#include <Windows.h>
#include <exception>
#include <expected>
#include <string>

struct SDL_Window;
struct SDL_Renderer;
struct SDL_Texture;

namespace fd
{
    enum engine_event
    {
        quit
    };

    enum input_event
    {
        key_down,
        key_up
    };

    struct event_stream_t;

    using error_t = std::exception;


    using window_t = void*;

    struct platform_t
    {
        std::expected<void, fd::error_t>(*init)();
        void (*exit)();
        void (*update)();

        std::expected<window_t, fd::error_t>(*create_window)(int width, int height, const char* title);
        void(*destroy_window)(window_t window);

        event_stream_t* (*get_engine_events)();

        SDL_Window* (*get_sdl_window)(window_t window);
        SDL_Renderer* (*get_sdl_renderer)(window_t window);
    };

    struct input_t
    {
        event_stream_t* (*get_input_events)();

        bool (*is_key_pressed)(int key_code);
    };



    struct jobdecl_t
    {
        void (*task)(void* data);
        void* data;
    };

    using jobs_handle_t = void*;

    struct job_system_t
    {
        void (*init)();
        void (*deinit)();
        jobs_handle_t(*run_jobs)(std::initializer_list<jobdecl_t> jobs);
        void (*wait_for_counter)(jobs_handle_t handle, std::uint32_t value);
        void (*wait_for_counter_no_fiber)(jobs_handle_t handle, uint32_t value);
    };


    using texture_t = std::uint64_t;

    struct sprite_manager_t
    {
        std::expected<texture_t, error_t>(*load_sprite)(std::string path, window_t window);
        SDL_Texture* (*texture)(texture_t handle);
    };
}