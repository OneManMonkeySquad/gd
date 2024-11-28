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
};

struct bird {};
struct pipe {};

struct position {
	float2 value;
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

std::expected<update_result, error> game_fixed_update(engine& engine, entt::registry& registry)
{
	auto& ctx = registry.ctx();
	auto& game = ctx.get<::game>();

	if (game.state == game_state::none)
	{
		auto birdTexture = engine.sprite_manager->load_sprite("sprites\\bluebird-downflap.png");
		auto pipeTexture = engine.sprite_manager->load_sprite("sprites\\pipe-red.png");

		auto birdEntity = registry.create();
		registry.emplace<position>(birdEntity, float2{ 288 * 0.5f, 512 * 0.5f });
		registry.emplace<sprite>(birdEntity, *birdTexture, float2{ 64,64 });
		registry.emplace<velocity>(birdEntity, float2{ 0, 0 });
		registry.emplace<bird>(birdEntity);

		auto pipeEntity = registry.create();
		registry.emplace<position>(pipeEntity, float2{ 70.f, 150.f });
		registry.emplace<sprite>(pipeEntity, *pipeTexture, float2{ 64,128 });
		registry.emplace<pipe>(pipeEntity);

		game.state = game_state::running;
	}
	else if (game.state == game_state::running)
	{
		bool birdFlap = false;

		// handle events
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

		// simulate
		registry.view<bird, velocity>().each([&](auto& velo) {
			velo.linear.y = birdFlap ? 9000 : 0;
			});

		auto birdView = registry.view<bird, velocity>();
		for (auto entity : birdView) {
			auto [velocity] = birdView.get(entity);
			velocity.linear.y -= 0.3f;
		}

		registry.view<velocity, position>().each([](velocity& velocity, position& position) {
			auto dt = (1.f / 30.f) * 0.5f;
			position.value += velocity.linear * dt;
			velocity.linear *= 1 - dt;
			});

		bool lost = false;
		registry.view<bird, position>().each([&](auto& pos) {
			if (pos.value.y < -100)
			{
				lost = true;
			}
			});

		if (lost)
		{
			game.state = game_state::lost;
		}

		registry.view<pipe, position>().each([&](auto& pos) {
			pos.value.x -= 0.01f;
			if (pos.value.x < -60)
			{
				pos.value.x = 300;
			}
			});
	}
	else if (game.state == game_state::lost)
	{
		registry.clear(); // reset game
		game.state = game_state::none;
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
		auto bgTexture = engine.sprite_manager->load_sprite("sprites\\background-day.png");

		SDL_FRect dst_rect;
		dst_rect.x = 0;
		dst_rect.y = 0;
		dst_rect.w = 288;
		dst_rect.h = 512;
		SDL_RenderTexture(engine.renderer, engine.sprite_manager->textures[*bgTexture], NULL, &dst_rect);
	}

	// draw debug text
	SDL_SetRenderDrawColor(engine.renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);
	SDL_RenderDebugText(engine.renderer, 5, 5, std::to_string((int)game.state).c_str());

	// draw sprites
	auto rendableView = registry.view<position, sprite>();
	rendableView.each([&](auto& pos, auto& sprite)
		{
			SDL_FRect dst_rect;
			dst_rect.x = pos.value.x;
			dst_rect.y = windowHeight - pos.value.y;
			dst_rect.w = sprite.size.x;
			dst_rect.h = sprite.size.y;
			SDL_RenderTexture(engine.renderer, engine.sprite_manager->textures[sprite.texture], NULL, &dst_rect);
		});

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
		const float timeStep = 1.f / 30.f;
		while (!quit) {
			Uint64 time = SDL_GetPerformanceCounter();
			float secondsElapsed = (time - lastTime) / (float)SDL_GetPerformanceFrequency();
			if (secondsElapsed > 0.25f)
			{
				secondsElapsed = 0.25f;
			}

			accTime += secondsElapsed;

			while (accTime >= timeStep)
			{
				accTime -= timeStep;
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