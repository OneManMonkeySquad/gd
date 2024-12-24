#include "rpg_game.h"
#include "foundation\engine_math.h"
#include "foundation\foundation.h"
#include "foundation\api_registry.h"
#include "foundation\event_stream.h"
#include "foundation\print.h"
#include <Windows.h>
#include <SDL3/SDL.h>

namespace fs = std::filesystem;

enum class game_state {
    none,
    running,
};

struct game_t {
    game_state state;
};

struct player_t {};
struct camera_t {};

struct transform_t {
    fd::float2 position;
    float angle;
};

struct sprite_t {
    fd::texture_t texture;
    fd::float2 size;
    fd::float2 texture_offset;
};

const auto fixed_time_step = 1.f / 30.f;

namespace
{
    fd::platform_t* platform;
    fd::input_t* input;
    fd::sprite_manager_t* sprite_manager;
    fd::window_t window;
    registry_t registry;
    std::uint64_t lastTime = 0;
    float accTime = 0;

    std::expected<update_result, fd::error_t> game_fixed_update(registry_t& registry);
    void game_render(const registry_t& registry);

    void start()
    {
        window = *platform->create_window(800, 600, "RPG");

        auto& ctx = registry.ctx();
        ctx.emplace<::game_t>();
    }

    update_result run_once()
    {
        //
        auto evts = platform->get_engine_events();
        for (auto evt_pair : *evts)
        {
            if (evt_pair.first == fd::engine_event::quit)
                return update_result::quit;
        }

        if (input->is_key_pressed(SDL_SCANCODE_ESCAPE))
            return update_result::quit;

        //
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
                return update_result::quit;
        }

        // #todo interpolate rendering
        game_render(registry);

        return update_result::keep_running;
    }

    void exit()
    {
        platform->destroy_window(window);
    }

    std::expected<update_result, fd::error_t> game_fixed_update(registry_t& registry)
    {
        auto& ctx = registry.ctx();
        auto& game = ctx.get<::game_t>();

        if (game.state == game_state::none)
        {
            auto player_texture = sprite_manager->load_sprite("sprites\\entities.bmp", window);

            auto camera_entity = registry.create();
            registry.emplace<transform_t>(camera_entity, fd::float2{ 288 * 0.5f, 512 * 0.5f });
            registry.emplace<camera_t>(camera_entity);

            auto player_entity = registry.create();
            registry.emplace<transform_t>(player_entity, fd::float2{ 288 * 0.5f, 512 * 0.5f });
            registry.emplace<sprite_t>(player_entity, *player_texture, fd::float2{ 16,16 }, fd::float2{ 0,0 });
            registry.emplace<player_t>(player_entity);

            game.state = game_state::running;
        }
        else if (game.state == game_state::running)
        {
            // simulate
            fd::float2 player_pos = fd::float2::zero;

            registry.view<player_t, transform_t>().each([&](auto& tf) {
                if (input->is_key_pressed(SDL_SCANCODE_W))
                {
                    tf.position.y += 4;
                }
                if (input->is_key_pressed(SDL_SCANCODE_S))
                {
                    tf.position.y -= 4;
                }
                if (input->is_key_pressed(SDL_SCANCODE_A))
                {
                    tf.position.x -= 4;
                }
                if (input->is_key_pressed(SDL_SCANCODE_D))
                {
                    tf.position.x += 4;
                }

                player_pos = tf.position;
                });

            registry.view<camera_t, transform_t>().each([&](auto& tf) {
                tf.position = fd::lerp(tf.position, player_pos, 0.1f);
                });
        }

        return update_result::keep_running;
    }

    void game_render(const registry_t& registry)
    {
        auto& ctx = registry.ctx();
        auto& game = ctx.get<::game_t>();

        int window_width;
        int window_height;
        if (!SDL_GetWindowSize(platform->get_sdl_window(window), &window_width, &window_height))
            throw fd::error_t(SDL_GetError());

        // clear
        SDL_SetRenderDrawColor(platform->get_sdl_renderer(window), 80, 80, 80, SDL_ALPHA_OPAQUE);
        SDL_RenderClear(platform->get_sdl_renderer(window));

        // draw debug text
        // SDL_SetRenderDrawColor(platform->renderer(window), 255, 255, 255, SDL_ALPHA_OPAQUE);
        // SDL_RenderDebugText(platform->renderer(window), 5, 5, std::to_string((int)game.state).c_str());

        // draw sprites
        auto camera_view = registry.view<transform_t, camera_t>();
        camera_view.each([&](auto& camera_tf) {
            auto pixel_scale = 4;

            // note: the camera transform positon is the center of the camera
            auto camera_pos = camera_tf.position - fd::float2(window_width * 0.5f, window_height * 0.5f);

            // Map
            {
                auto texture = *sprite_manager->load_sprite("sprites\\world.bmp", window);

                for (int x = 0; x < 14; ++x)
                {
                    for (int y = 0; y < 14; ++y)
                    {
                        SDL_FRect src_rect;
                        src_rect.x = ((x + y * 133502863135036867) % 2) * 16;
                        src_rect.y = 0;
                        src_rect.w = 16;
                        src_rect.h = 16;

                        SDL_FRect dst_rect;
                        dst_rect.x = 16 * x * pixel_scale - camera_pos.x;
                        dst_rect.y = 16 * y * pixel_scale + camera_pos.y;
                        dst_rect.w = 16 * pixel_scale;
                        dst_rect.h = 16 * pixel_scale;

                        if (!SDL_RenderTextureRotated(platform->get_sdl_renderer(window), sprite_manager->texture(texture), &src_rect, &dst_rect, 0, nullptr, SDL_FLIP_NONE))
                            throw fd::error_t(SDL_GetError());
                    }
                }
            }

            // Sprites
            auto rendable_view = registry.view<transform_t, sprite_t>();
            rendable_view.each([&](auto& pos, auto& sprite) {
                SDL_FRect src_rect;
                src_rect.x = sprite.texture_offset.x;
                src_rect.y = sprite.texture_offset.y;
                src_rect.w = sprite.size.x;
                src_rect.h = sprite.size.y;

                SDL_FRect dst_rect;
                dst_rect.x = (pos.position.x - sprite.size.x * 0.5f) - camera_pos.x;
                dst_rect.y = ((window_height - pos.position.y) - sprite.size.y * 0.5f) + camera_pos.y;
                dst_rect.w = sprite.size.x * pixel_scale;
                dst_rect.h = sprite.size.y * pixel_scale;

                auto texture = sprite_manager->texture(sprite.texture);

                if (!SDL_RenderTextureRotated(platform->get_sdl_renderer(window), texture, &src_rect, &dst_rect, pos.angle, nullptr, SDL_FLIP_NONE))
                    throw fd::error_t(SDL_GetError());
                });

            });

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
    input = api.get<fd::input_t>();
    sprite_manager = api.get<fd::sprite_manager_t>();

    if (reload)
    {
        auto existing_game = api.get<rpg_game_t>();

        fd::println("transfer state from old game to new game...");
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