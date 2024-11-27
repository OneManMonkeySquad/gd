#include <format>
#include <expected>
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <entt/entt.hpp>

using texture_handle = size_t;

enum class error_code {
	unknown,
};

struct error {
	error_code code = error_code::unknown;
	const char* message;
};

struct sprite_mananger {
	SDL_Renderer* renderer = nullptr;
	std::vector<SDL_Texture*> textures;

	sprite_mananger(SDL_Renderer* renderer)
		: renderer(renderer)
	{ }

	std::expected<texture_handle, error> load_sprite(std::string path)
	{
		SDL_Surface* surface = IMG_Load(path.c_str());

		//SDL_Surface* surface = SDL_LoadBMP(path.c_str());
		if (!surface)
			return std::unexpected(error{ .message = SDL_GetError() });

		SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
		if (!texture)
			return std::unexpected(error{ .message = SDL_GetError() });

		SDL_DestroySurface(surface);

		auto textureIdx = textures.size();
		textures.push_back(texture);
		return textureIdx;
	}
};

struct game {
	SDL_Window* window = nullptr;
	SDL_Renderer* renderer = nullptr;
	sprite_mananger* sprite_manager = nullptr;
};

struct bird {};

struct position {
	float x;
	float y;
};

struct sprite {
	texture_handle texture;
};

int init(game& game)
{
	if (!SDL_Init(SDL_INIT_VIDEO)) {
		SDL_Log("SDL_Init failed (%s)", SDL_GetError());
		return 1;
	}

	if (!SDL_CreateWindowAndRenderer("SDL issue", 800, 600, 0, &game.window, &game.renderer)) {
		SDL_Log("SDL_CreateWindowAndRenderer failed (%s)", SDL_GetError());
		SDL_Quit();
		return 1;
	}

	game.sprite_manager = new sprite_mananger(game.renderer);
	return 0;
}

void deinit(game& game)
{
	delete game.sprite_manager;
	game.sprite_manager = nullptr;
	SDL_DestroyRenderer(game.renderer);
	game.renderer = nullptr;
	SDL_DestroyWindow(game.window);
	game.window = nullptr;
	SDL_Quit();
}

void render(game& game, entt::registry& registry)
{
	SDL_SetRenderDrawColor(game.renderer, 80, 80, 80, SDL_ALPHA_OPAQUE);
	SDL_RenderClear(game.renderer);

	SDL_SetRenderDrawColor(game.renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);
	SDL_RenderDebugText(game.renderer, 5, 5, "Hello world!");

	auto rendable = registry.view<position, sprite>();
	rendable.each([&](auto& pos, auto& sprite) 
		{
			SDL_FRect dst_rect;
			dst_rect.x = pos.x;
			dst_rect.y = pos.y;
			dst_rect.w = 64;
			dst_rect.h = 64;
			SDL_RenderTexture(game.renderer, game.sprite_manager->textures[sprite.texture], NULL, &dst_rect);
		});

	SDL_RenderPresent(game.renderer);
}

int main(int argc, char* argv[]) {
	(void)argc;
	(void)argv;

	game game;
	int result = init(game);
	if (result != 0)
		return result;

	entt::registry registry;

	auto textureIdx = game.sprite_manager->load_sprite(std::format("{}\\data\\sprites\\bluebird-downflap.png", SDL_GetBasePath()));

	auto birdEntity = registry.create();
	registry.emplace<position>(birdEntity, 0.f, 0.f);
	registry.emplace<sprite>(birdEntity, *textureIdx);
	registry.emplace<bird>(birdEntity);

	auto entity2 = registry.create();
	registry.emplace<position>(entity2, 70.f, 30.f);
	registry.emplace<sprite>(entity2, *textureIdx);

	while (1) {
		int finished = 0;
		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_EVENT_QUIT) {
				finished = 1;
				break;
			}
			if (event.type == SDL_EVENT_KEY_DOWN)
			{
				if (event.key.scancode == SDL_SCANCODE_ESCAPE)
				{
					finished = true;
				}
			}
		}
		if (finished)
			break;

		render(game, registry);
	}

	deinit(game);
	return 0;
}