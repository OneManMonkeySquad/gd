module;

#include <Windows.h>

export module foundation;

import std;
import <SDL3/SDL.h>;

export import :math;
export import :global_exception_handler;
export import :print;
export import :plugin_manager;
export import :api_registry;

namespace foundation
{
    export enum class error_code {
        unknown,
    };

    export struct error_t {
        error_code code = error_code::unknown;
        const char* message;
    };

    using window_t = void*;

    export struct engine_t
    {
        window_t(*create_window)(int width, int height, const char* title);
        void(*destroy_window)(window_t window);

        std::expected<void, error_t>(*init)(int width, int height, const char* title);
        void (*deinit)();
        SDL_Window* (*window)();
        SDL_Renderer* (*renderer)();
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


    export using texture_handle_t = size_t;

    export struct sprite_manager_t
    {
        std::expected<texture_handle_t, error_t>(*load_sprite)(std::string path);
        SDL_Texture* (*texture)(texture_handle_t handle);
    };


    export struct game_t
    {
        void (*start)();
        bool (*run_once)();
        void (*exit)();
    };
}