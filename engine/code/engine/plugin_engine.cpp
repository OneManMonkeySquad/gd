import foundation;
import std;
import <SDL3/SDL.h>;

class MyEngine : public foundation::IEngine
{
public:
    SDL_Window* _window = nullptr;
    SDL_Renderer* _renderer = nullptr;

    std::expected<void, foundation::error> init(int width, int height, const char* title) override
    {
        foundation::println("MyEngine::init 2");

        if (!SDL_Init(SDL_INIT_VIDEO))
            return std::unexpected(foundation::error{ .message = SDL_GetError() });

        if (!SDL_CreateWindowAndRenderer(title, width, height, 0, &_window, &_renderer))
            return std::unexpected(foundation::error{ .message = SDL_GetError() });

        if (!SDL_SetRenderVSync(_renderer, 1)) // always vsync for now to keep framerate sane
            return std::unexpected(foundation::error{ .message = SDL_GetError() });

        return {};
    }

    void deinit() override
    {
        SDL_DestroyRenderer(_renderer);
        _renderer = nullptr;
        SDL_DestroyWindow(_window);
        _window = nullptr;
        SDL_Quit();
    }

    SDL_Window* window() override
    {
        return _window;
    }
    SDL_Renderer* renderer() override
    {
        return _renderer;
    }
} engine;

extern "C" __declspec(dllexport) void plugin_load(foundation::api_registry& api, bool reload)
{
    if (reload)
    {
        auto existing_engine = (MyEngine*)api.get<foundation::IEngine>().raw_ptr();
        engine._window = existing_engine->_window;
        engine._renderer = existing_engine->_renderer;
    }

    api.set<foundation::IEngine>(&engine);
}

extern "C" __declspec(dllexport) void plugin_unload(foundation::api_registry& api, bool reload)
{
    api.remove(&engine);
}