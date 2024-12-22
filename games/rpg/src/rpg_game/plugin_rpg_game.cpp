#include "rpg_game.h"
#include "foundation\engine_math.h"
#include "foundation\foundation.h"
#include "foundation\api_registry.h"
#include <Windows.h>
#include <SDL3/SDL.h>

namespace fs = std::filesystem;

enum class game_state {
    none,
    running,
    lost
};

struct game {
    game_state state;
    float death_time;
};

struct bird_t {};

struct transform_t {
    fd::float2 value;
    float angle;
};

struct velocity_t {
    fd::float2 linear;
};

struct sprite_t {
    fd::texture_t texture;
    fd::float2 size;
};

enum update_result {
    keep_running,
    quit
};



const auto fixed_time_step = 1.f / 30.f;

namespace
{
    fd::platform_t* platform;
    fd::sprite_manager_t* sprite_manager;
    fd::window_t window;
    registry_t registry;
    std::uint64_t lastTime = 0;
    float accTime = 0;

    std::expected<update_result, fd::error_t> game_fixed_update(registry_t& registry);
    void game_render(const registry_t& registry);

    void start()
    {
        window = *platform->create_window(512, 512, "RPG");

        auto& ctx = registry.ctx();
        ctx.emplace<::game>();
    }

    bool run_once()
    {
        std::uint64_t time = SDL_GetPerformanceCounter();
        float seconds_elapsed = (time - lastTime) / (float)SDL_GetPerformanceFrequency();
        if (seconds_elapsed > 0.25f)
        {
            seconds_elapsed = 0.25f;
        }
        lastTime = time;

        accTime += seconds_elapsed;

        while (accTime >= fixed_time_step)
        {
            accTime -= fixed_time_step;
            if (game_fixed_update(registry) != update_result::keep_running)
                return false;
        }

        // #todo interpolate rendering
        game_render(registry);

        return true;
    }

    void exit()
    {
        platform->destroy_window(window);
    }

    std::expected<update_result, fd::error_t> game_fixed_update(registry_t& registry)
    {
        auto& ctx = registry.ctx();
        auto& game = ctx.get<::game>();

        // handle events
        bool bird_flap = false;

        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT)
                return update_result::quit;

            if (event.type == SDL_EVENT_KEY_DOWN)
            {
                if (event.key.scancode == SDL_SCANCODE_ESCAPE)
                    return update_result::quit;

                if (event.key.scancode == SDL_SCANCODE_SPACE)
                {
                    bird_flap = true;
                }
            }
        }

        if (game.state == game_state::none)
        {
            auto bird_texture = sprite_manager->load_sprite("sprites\\bluebird-downflap.png", window);

            auto bird_entity = registry.create();
            registry.emplace<transform_t>(bird_entity, fd::float2{ 288 * 0.5f, 512 * 0.5f });
            registry.emplace<sprite_t>(bird_entity, *bird_texture, fd::float2{ 64,64 });
            registry.emplace<velocity_t>(bird_entity, fd::float2{ 0, 0 });
            registry.emplace<bird_t>(bird_entity);

            game.state = game_state::running;
        }
        else if (game.state == game_state::running)
        {
            // simulate
            registry.view<bird_t, velocity_t>().each([&](auto& velo) {
                velo.linear.y += bird_flap ? 4 : 0;
                });

            auto bird_view = registry.view<bird_t, velocity_t>();
            for (auto entity : bird_view) {
                auto [velocity] = bird_view.get(entity);
                velocity.linear.y -= 0.02f; // <-------------------------------------------------------------------------------------------
            }

            registry.view<velocity_t, transform_t>().each([](auto& velocity, auto& position) {
                position.value += velocity.linear;
                });

            bool lost = false;
            fd::float2 bird_pos;
            registry.view<bird_t, transform_t>().each([&](auto& tf) {
                bird_pos = tf.value;
                if (tf.value.y < 0 || tf.value.y > 500)
                {
                    lost = true;
                }
                });

            if (lost)
            {
                // fd::println("lost game");
                game.state = game_state::lost;
                game.death_time = 2;
            }
        }
        else if (game.state == game_state::lost)
        {
            game.death_time -= fixed_time_step;
            if (game.death_time <= 0)
            {
                // fd::println("restart game");

                registry.clear(); // reset game
                game.state = game_state::none;
            }
        }

        return update_result::keep_running;
    }

    void game_render(const registry_t& registry)
    {
        auto& ctx = registry.ctx();
        auto& game = ctx.get<::game>();

        int window_height;
        if (!SDL_GetWindowSize(platform->get_sdl_window(window), nullptr, &window_height))
            throw fd::error_t(SDL_GetError());

        // clear
        SDL_SetRenderDrawColor(platform->get_sdl_renderer(window), 80, 80, 80, SDL_ALPHA_OPAQUE);
        SDL_RenderClear(platform->get_sdl_renderer(window));

        {
            auto texture = *sprite_manager->load_sprite("sprites\\world.bmp", window);

            SDL_FRect src_rect;
            src_rect.x = 0;
            src_rect.y = 0;
            src_rect.w = 16;
            src_rect.h = 16;

            SDL_FRect dst_rect;
            dst_rect.x = 30;
            dst_rect.y = 30;
            dst_rect.w = 30;
            dst_rect.h = 30;

            if (!SDL_RenderTexture(platform->get_sdl_renderer(window), sprite_manager->texture(texture), &src_rect, &dst_rect))
                throw fd::error_t(SDL_GetError());
        }

        // draw debug text
        // SDL_SetRenderDrawColor(platform->renderer(window), 255, 255, 255, SDL_ALPHA_OPAQUE);
        // SDL_RenderDebugText(platform->renderer(window), 5, 5, std::to_string((int)game.state).c_str());

        // draw sprites
        auto rendable_view = registry.view<transform_t, sprite_t>();
        rendable_view.each([&](auto& pos, auto& sprite) {
            SDL_FRect dst_rect;
            dst_rect.x = pos.value.x - sprite.size.x * 0.5f;
            dst_rect.y = (window_height - pos.value.y) - sprite.size.y * 0.5f;
            dst_rect.w = sprite.size.x;
            dst_rect.h = sprite.size.y;

            auto texture = sprite_manager->texture(sprite.texture);

            if (!SDL_RenderTextureRotated(platform->get_sdl_renderer(window), texture, nullptr, &dst_rect, pos.angle, nullptr, SDL_FLIP_NONE))
                throw fd::error_t(SDL_GetError());
            });

        // present
        if (!SDL_RenderPresent(platform->get_sdl_renderer(window)))
            throw fd::error_t(SDL_GetError());
    }
}

extern "C" __declspec(dllexport) void set_plugin_runtime(entt::locator<entt::meta_ctx>::node_type handle)
{
    entt::locator<entt::meta_ctx>::reset(handle);
}



extern "C" __declspec(dllexport) state_t get_state()
{
    state_t foo;
    foo.p1 = window;
    foo.p2 = &registry;
    return foo;
}



extern "C" __declspec(dllexport) void load_plugin(fd::api_registry_t& api, bool reload, void* old_dll)
{
    platform = api.get<fd::platform_t>();
    sprite_manager = api.get<fd::sprite_manager_t>();

    if (reload)
    {
        auto existing_game = api.get<rpg_game_t>();

        auto foo = existing_game->get_state();
        window = foo.p1;
        registry = std::move(*foo.p2);
    }

    rpg_game_t game;
    game.get_state = get_state;
    game.start = start;
    game.run_once = run_once;
    game.exit = exit;

    api.set(game);
}

extern "C" __declspec(dllexport) void unload_plugin(fd::api_registry_t& api, bool reload)
{
    if (reload)
    {
    }

    // api.remove(foo);
    // delete foo;
}