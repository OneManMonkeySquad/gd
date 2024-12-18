import foundation;
import std;
import <SDL3/SDL.h>;

namespace
{
    SDL_Window* _window = nullptr;
    SDL_Renderer* _renderer = nullptr;

    std::expected<void, foundation::error_t> init(int width, int height, const char* title)
    {
        if (!SDL_Init(SDL_INIT_VIDEO))
            return std::unexpected(foundation::error_t{ .message = SDL_GetError() });

        if (!SDL_CreateWindowAndRenderer(title, width, height, 0, &_window, &_renderer))
            return std::unexpected(foundation::error_t{ .message = SDL_GetError() });

        if (!SDL_SetRenderVSync(_renderer, 1)) // always vsync for now to keep framerate sane
            return std::unexpected(foundation::error_t{ .message = SDL_GetError() });

        return {};
    }

    void deinit()
    {
        SDL_DestroyRenderer(_renderer);
        _renderer = nullptr;
        SDL_DestroyWindow(_window);
        _window = nullptr;
        SDL_Quit();
    }

    SDL_Window* window()
    {
        return _window;
    }
    SDL_Renderer* renderer()
    {
        return _renderer;
    }
}

extern "C" __declspec(dllexport) void plugin_load(foundation::api_registry& api, bool reload)
{
    if (reload)
    {
        // auto existing_engine = (MyEngine*)api.get<foundation::IEngine>().raw_ptr();
        // engine._window = existing_engine->_window;
        // engine._renderer = existing_engine->_renderer;
    }

    foundation::engine_t e;
    e.init = &init;
    e.deinit = &deinit;
    e.renderer = &renderer;
    e.window = &window;

    api.set(e);
}

extern "C" __declspec(dllexport) void plugin_unload(foundation::api_registry& api, bool reload)
{
    // api.remove(&engine);
}