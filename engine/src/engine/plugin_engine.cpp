#include "foundation/foundation.h"
#include "foundation/api_registry.h"
#include "foundation/event_stream.h"
#include <SDL3/SDL.h>

namespace
{
    struct my_window_t
    {
        SDL_Window* window = nullptr;
        SDL_Renderer* renderer = nullptr;
    };

    fd::event_stream_t engine_events;
    fd::event_stream_t input_events;

    std::expected<void, fd::error_t> init()
    {
        if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD))
            return std::unexpected(fd::error_t(SDL_GetError()));

        return{};
    }

    void exit()
    {
        SDL_Quit();
    }

    std::expected<fd::window_t, fd::error_t> create_window(int width, int height, const char* title)
    {
        auto ptr = std::make_unique<my_window_t>();

        if (!SDL_CreateWindowAndRenderer(title, width, height, 0, &ptr->window, &ptr->renderer))
            return std::unexpected(fd::error_t(SDL_GetError()));

        if (!SDL_SetRenderVSync(ptr->renderer, 1)) // always vsync for now to keep framerate sane
            return std::unexpected(fd::error_t(SDL_GetError()));

        return (fd::window_t*)ptr.release();
    }

    void destroy_window(fd::window_t window)
    {
        auto my_window = (my_window_t*)window;

        SDL_DestroyRenderer(my_window->renderer);
        SDL_DestroyWindow(my_window->window);

        delete my_window;
    }

    fd::event_stream_t* get_engine_events()
    {
        return &engine_events;
    }

    void update()
    {
        engine_events.clear();
        input_events.clear();

        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT)
            {
                engine_events.append(fd::engine_event::quit);
            }
            else if (event.type == SDL_EVENT_KEY_DOWN)
            {
                input_events.append(fd::input_event::key_down);
            }
            else if (event.type == SDL_EVENT_KEY_UP)
            {
                input_events.append(fd::input_event::key_up);
            }
        }
    }

    SDL_Window* get_sdl_window(fd::window_t window)
    {
        auto my_window = (my_window_t*)window;
        return my_window->window;
    }
    SDL_Renderer* get_sdl_renderer(fd::window_t window)
    {
        auto my_window = (my_window_t*)window;
        return my_window->renderer;
    }

    fd::event_stream_t* get_input_events()
    {
        return &input_events;
    }

    bool is_key_pressed(int key_code)
    {
        const bool* state = SDL_GetKeyboardState(nullptr);
        return state[key_code];
    }
}

extern "C" __declspec(dllexport) void load_plugin(fd::api_registry_t& api, bool reload, void* old_dll)
{
    if (reload)
    {
        // auto existing_engine = (MyEngine*)api.get<fd::IEngine>().raw_ptr();
        // engine._window = existing_engine->_window;
        // engine._renderer = existing_engine->_renderer;
    }

    fd::platform_t platform;
    platform.init = init;
    platform.exit = exit;
    platform.update = update;
    platform.create_window = create_window;
    platform.destroy_window = destroy_window;
    platform.get_sdl_renderer = get_sdl_renderer;
    platform.get_sdl_window = get_sdl_window;
    platform.get_engine_events = get_engine_events;
    api.set(platform);

    fd::input_t input;
    input.get_input_events = get_input_events;
    input.is_key_pressed = is_key_pressed;
    api.set(input);
}

extern "C" __declspec(dllexport) void unload_plugin(fd::api_registry_t& api, bool reload)
{
    api.reset<fd::platform_t>();
    api.reset<fd::input_t>();
}