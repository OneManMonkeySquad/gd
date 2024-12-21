module;

#include <Windows.h>

export module foundation;

import std;
import <SDL3/SDL.h>;

export import :math;
export import :global_exception_handler;
export import :print;
export import :plugin_manager;
export import :api_registry_t;

namespace fd
{
    export using error_t = std::exception;


    export using window_t = void*;

    export struct platform_t
    {
        std::expected<void, fd::error_t>(*init)();
        void (*exit)();

        std::expected<window_t, fd::error_t>(*create_window)(int width, int height, const char* title);
        void(*destroy_window)(window_t window);

        SDL_Window* (*get_sdl_window)(window_t window);
        SDL_Renderer* (*get_sdl_renderer)(window_t window);
    };



    export struct jobdecl_t
    {
        void (*task)(void* data);
        void* data;
    };

    export using jobs_handle_t = void*;

    export struct job_system_t
    {
        void (*init)();
        void (*deinit)();
        jobs_handle_t(*run_jobs)(std::initializer_list<jobdecl_t> jobs);
        void (*wait_for_counter)(jobs_handle_t handle, uint32_t value);
        void (*wait_for_counter_no_fiber)(jobs_handle_t handle, uint32_t value);
    };


    export using texture_t = std::uint64_t;

    export struct sprite_manager_t
    {
        std::expected<texture_t, error_t>(*load_sprite)(std::string path, window_t window);
        SDL_Texture* (*texture)(texture_t handle);
    };


    export struct game_t
    {
        void (*start)();
        bool (*run_once)();
        void (*exit)();
    };
}