import foundation;
import <SDL3/SDL.h>;
import <entt/entt.hpp>;
import <nlohmann/json.hpp>;
import std;

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
struct pipe_t {};

struct transform_t {
    foundation::float2 value;
    float angle;
};

struct velocity_t {
    foundation::float2 linear;
};

struct sprite_t {
    foundation::texture_handle_t texture;
    foundation::float2 size;
};

enum update_result {
    keep_running,
    quit
};



const auto fixed_time_step = 1.f / 30.f;

namespace
{
    foundation::engine_t* engine;
    foundation::sprite_manager_t* sprite_manager;
    entt::registry registry;
    Uint64 lastTime = 0;
    float accTime = 0;

    std::expected<update_result, foundation::error_t> game_fixed_update(entt::registry& registry);
    std::expected<void, foundation::error_t> game_render(const entt::registry& registry);

    void start()
    {
        engine->init(288, 512, "Flappy");

        auto& ctx = registry.ctx();
        ctx.emplace<::game>();
    }

    bool run_once()
    {
        Uint64 time = SDL_GetPerformanceCounter();
        float secondsElapsed = (time - lastTime) / (float)SDL_GetPerformanceFrequency();
        if (secondsElapsed > 0.25f)
        {
            secondsElapsed = 0.25f;
        }
        lastTime = time;

        accTime += secondsElapsed;

        while (accTime >= fixed_time_step)
        {
            accTime -= fixed_time_step;
            if (game_fixed_update(registry) != update_result::keep_running)
                return false;
        }

        // #todo interpolate rendering
        *game_render(registry);

        return true;
    }

    void exit()
    {
        engine->deinit();
    }

    std::expected<update_result, foundation::error_t> game_fixed_update(entt::registry& registry)
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
            auto birdTexture = sprite_manager->load_sprite("sprites\\bluebird-downflap.png");
            auto pipeTexture = sprite_manager->load_sprite("sprites\\pipe-red.png");

            auto birdEntity = registry.create();
            registry.emplace<transform_t>(birdEntity, foundation::float2{ 288 * 0.5f, 512 * 0.5f });
            registry.emplace<sprite_t>(birdEntity, *birdTexture, foundation::float2{ 64,64 });
            registry.emplace<velocity_t>(birdEntity, foundation::float2{ 0, 0 });
            registry.emplace<bird_t>(birdEntity);

            auto pipeEntity = registry.create();
            registry.emplace<transform_t>(pipeEntity, foundation::float2{ 70.f, 220 * 0.5f });
            registry.emplace<sprite_t>(pipeEntity, *pipeTexture, foundation::float2{ 64,220 });
            registry.emplace<pipe_t>(pipeEntity);

            auto pipe2Entity = registry.create();
            registry.emplace<transform_t>(pipe2Entity, foundation::float2{ 70.f, 512 - 220 * 0.5f }, 180.f);
            registry.emplace<sprite_t>(pipe2Entity, *pipeTexture, foundation::float2{ 64,220 });
            registry.emplace<pipe_t>(pipe2Entity);

            game.state = game_state::running;
        }
        else if (game.state == game_state::running)
        {
            // simulate
            registry.view<bird_t, velocity_t>().each([&](auto& velo) {
                velo.linear.y += bird_flap ? 4 : 0;
                });

            auto birdView = registry.view<bird_t, velocity_t>();
            for (auto entity : birdView) {
                auto [velocity] = birdView.get(entity);
                velocity.linear.y -= 0.2f;
            }

            registry.view<velocity_t, transform_t>().each([](auto& velocity, auto& position) {
                position.value += velocity.linear;
                });

            bool lost = false;
            foundation::float2 birdPos;
            registry.view<bird_t, transform_t>().each([&](auto& tf) {
                birdPos = tf.value;
                if (tf.value.y < 0 || tf.value.y > 500)
                {
                    lost = true;
                }
                });

            registry.view<pipe_t, transform_t>().each([&](auto& tf) {
                tf.value.x -= 5;
                if (tf.value.x < -60)
                {
                    tf.value.x = 300;
                }

                SDL_FPoint birdP{ birdPos.x, birdPos.y };
                SDL_FRect rect{ tf.value.x - 32, tf.value.y - 110, 64, 220 };
                if (SDL_PointInRectFloat(&birdP, &rect))
                {
                    lost = true;
                }
                });

            if (lost)
            {
                game.state = game_state::lost;
                game.death_time = 2;
            }
        }
        else if (game.state == game_state::lost)
        {
            game.death_time -= fixed_time_step;
            if (game.death_time <= 0)
            {
                registry.clear(); // reset game
                game.state = game_state::none;
            }
        }

        return update_result::keep_running;
    }

    std::expected<void, foundation::error_t> game_render(const entt::registry& registry)
    {
        auto& ctx = registry.ctx();
        auto& game = ctx.get<::game>();

        int windowHeight;
        if (!SDL_GetWindowSize(engine->window(), nullptr, &windowHeight))
            return std::unexpected(foundation::error_t{ .message = SDL_GetError() });

        // clear
        SDL_SetRenderDrawColor(engine->renderer(), 80, 80, 80, SDL_ALPHA_OPAQUE);
        SDL_RenderClear(engine->renderer());

        // draw background
        {
            auto bgTexture = *sprite_manager->load_sprite("sprites\\background-day.png");

            SDL_FRect dst_rect;
            dst_rect.x = 0;
            dst_rect.y = 0;
            dst_rect.w = 288;
            dst_rect.h = 512;
            SDL_RenderTexture(engine->renderer(), sprite_manager->texture(bgTexture), NULL, &dst_rect);
        }

        // draw debug text
        // SDL_SetRenderDrawColor(engine->renderer(), 255, 255, 255, SDL_ALPHA_OPAQUE);
        // SDL_RenderDebugText(engine->renderer(), 5, 5, std::to_string((int)game.state).c_str());

        // draw sprites
        auto rendableView = registry.view<transform_t, sprite_t>();
        rendableView.each([&](auto& pos, auto& sprite) {
            SDL_FRect dst_rect;
            dst_rect.x = pos.value.x - sprite.size.x * 0.5f;
            dst_rect.y = (windowHeight - pos.value.y) - sprite.size.y * 0.5f;
            dst_rect.w = sprite.size.x;
            dst_rect.h = sprite.size.y;

            auto texture = sprite_manager->texture(sprite.texture);

            SDL_RenderTextureRotated(engine->renderer(), texture, nullptr, &dst_rect, pos.angle, nullptr, SDL_FLIP_NONE);
            });

        // draw game over
        if (game.state == game_state::lost)
        {
            auto bgTexture = *sprite_manager->load_sprite("sprites\\gameover.png");

            SDL_FRect dst_rect;
            dst_rect.x = 288 * 0.5f - 192 * 0.5f;
            dst_rect.y = 512 * 0.5f - 42 * 0.5f;
            dst_rect.w = 192;
            dst_rect.h = 42;
            SDL_RenderTexture(engine->renderer(), sprite_manager->texture(bgTexture), NULL, &dst_rect);
        }

        // present
        if (!SDL_RenderPresent(engine->renderer()))
            return std::unexpected(foundation::error_t{ .message = SDL_GetError() });

        return {};
    }
}

extern "C" __declspec(dllexport) void plugin_fix_runtime(entt::locator<entt::meta_ctx>::node_type handle)
{
    entt::locator<entt::meta_ctx>::reset(handle);
}

extern "C" __declspec(dllexport) void plugin_load(foundation::api_registry& api, bool reload)
{


    engine = api.get<foundation::engine_t>();
    sprite_manager = api.get<foundation::sprite_manager_t>();

    if (reload)
    {
        // foo->registry = std::move(existing_game->registry);
    }

    foundation::game_t game;
    game.start = &start;
    game.run_once = &run_once;
    game.exit = &exit;

    api.set(game);
}

extern "C" __declspec(dllexport) void plugin_unload(foundation::api_registry& api, bool reload)
{
    // api.remove(foo);
    // delete foo;
}