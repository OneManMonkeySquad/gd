export module game;

import <SDL3/SDL.h>;
import <entt/entt.hpp>;

import std;
import math;
import engine;

enum class game_state {
	none,
	running,
	lost
};

struct game {
	game_state state;
	float death_time;
};

struct bird {};
struct pipe {};

struct transform {
	float2 value;
	float angle;
};

struct velocity {
	float2 linear;
};

enum update_result {
	keep_running,
	quit
};

void game_start(engine& engine, entt::registry& registry)
{
	auto& ctx = registry.ctx();
	ctx.emplace<game>();
}

const auto fixed_time_step = 1.f / 30.f;

std::expected<update_result, error> game_fixed_update(engine& engine, entt::registry& registry)
{
	auto& ctx = registry.ctx();
	auto& game = ctx.get<::game>();

	// handle events
	bool birdFlap = false;

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
				birdFlap = true;
			}
		}
	}

	if (game.state == game_state::none)
	{
		auto birdTexture = engine.sprite_manager->load_sprite("sprites\\bluebird-downflap.png");
		auto pipeTexture = engine.sprite_manager->load_sprite("sprites\\pipe-red.png");

		auto birdEntity = registry.create();
		registry.emplace<transform>(birdEntity, float2{ 288 * 0.5f, 512 * 0.5f });
		registry.emplace<sprite>(birdEntity, *birdTexture, float2{ 64,64 });
		registry.emplace<velocity>(birdEntity, float2{ 0, 0 });
		registry.emplace<bird>(birdEntity);

		auto pipeEntity = registry.create();
		registry.emplace<transform>(pipeEntity, float2{ 70.f, 220 * 0.5f });
		registry.emplace<sprite>(pipeEntity, *pipeTexture, float2{ 64,220 });
		registry.emplace<pipe>(pipeEntity);

		auto pipe2Entity = registry.create();
		registry.emplace<transform>(pipe2Entity, float2{ 70.f, 512 - 220 * 0.5f }, 180.f);
		registry.emplace<sprite>(pipe2Entity, *pipeTexture, float2{ 64,220 });
		registry.emplace<pipe>(pipe2Entity);

		game.state = game_state::running;
	}
	else if (game.state == game_state::running)
	{
		// simulate
		registry.view<bird, velocity>().each([&](auto& velo) {
			velo.linear.y += birdFlap ? 8 : 0;
			});

		auto birdView = registry.view<bird, velocity>();
		for (auto entity : birdView) {
			auto [velocity] = birdView.get(entity);
			velocity.linear.y -= 0.7f;
		}

		registry.view<velocity, transform>().each([](auto& velocity, auto& position) {
			position.value += velocity.linear;
			});

		bool lost = false;
		float2 birdPos;
		registry.view<bird, transform>().each([&](auto& tf) {
			birdPos = tf.value;
			if (tf.value.y < 0 || tf.value.y > 500)
			{
				lost = true;
			}
			});

		registry.view<pipe, transform>().each([&](auto& tf) {
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

std::expected<void, error> game_render(engine& engine, const entt::registry& registry)
{
	auto& ctx = registry.ctx();
	auto& game = ctx.get<::game>();

	int windowHeight;
	if (!SDL_GetWindowSize(engine.window, nullptr, &windowHeight))
		return std::unexpected(error{ .message = SDL_GetError() });

	// clear
	SDL_SetRenderDrawColor(engine.renderer, 80, 80, 80, SDL_ALPHA_OPAQUE);
	SDL_RenderClear(engine.renderer);

	// draw background
	{
		auto bgTexture = *engine.sprite_manager->load_sprite("sprites\\background-day.png");

		SDL_FRect dst_rect;
		dst_rect.x = 0;
		dst_rect.y = 0;
		dst_rect.w = 288;
		dst_rect.h = 512;
		SDL_RenderTexture(engine.renderer, engine.sprite_manager->textures[bgTexture], NULL, &dst_rect);
	}

	// draw debug text
	SDL_SetRenderDrawColor(engine.renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);
	SDL_RenderDebugText(engine.renderer, 5, 5, std::to_string((int)game.state).c_str());

	// draw sprites
	auto rendableView = registry.view<transform, sprite>();
	rendableView.each([&](auto& pos, auto& sprite) {
		SDL_FRect dst_rect;
		dst_rect.x = pos.value.x - sprite.size.x * 0.5f;
		dst_rect.y = (windowHeight - pos.value.y) - sprite.size.y * 0.5f;
		dst_rect.w = sprite.size.x;
		dst_rect.h = sprite.size.y;

		auto texture = engine.sprite_manager->textures[sprite.texture];

		SDL_RenderTextureRotated(engine.renderer, texture, nullptr, &dst_rect, pos.angle, nullptr, SDL_FLIP_NONE);
		});

	// draw game over
	if (game.state == game_state::lost)
	{
		auto bgTexture = *engine.sprite_manager->load_sprite("sprites\\gameover.png");

		SDL_FRect dst_rect;
		dst_rect.x = 288 * 0.5f - 192 * 0.5f;
		dst_rect.y = 512 * 0.5f - 42 * 0.5f;
		dst_rect.w = 192;
		dst_rect.h = 42;
		SDL_RenderTexture(engine.renderer, engine.sprite_manager->textures[bgTexture], NULL, &dst_rect);
	}

	// present
	if (!SDL_RenderPresent(engine.renderer))
		return std::unexpected(error{ .message = SDL_GetError() });

	return {};
}

export void run_game()
{
	try
	{
		engine engine = *engine_init();

		entt::registry registry;
		game_start(engine, registry);

		bool quit = false;
		Uint64 lastTime = 0;
		float accTime = 0;
		while (!quit)
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
				if (game_fixed_update(engine, registry) != update_result::keep_running)
				{
					quit = true;
					break;
				}
			}

			// #todo interpolate rendering
			*game_render(engine, registry);
		}

		engine_deinit(engine);
	}
	catch (const std::exception& exception)
	{
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Fatal Error", exception.what(), nullptr);
	}
}