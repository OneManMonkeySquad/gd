export module engine;

import <SDL3/SDL.h>;
import <SDL3_image/SDL_image.h>;
import std;
import math;

export using texture_handle = size_t;

export enum class error_code {
	unknown,
};

export struct error {
	error_code code = error_code::unknown;
	const char* message;
};

export struct sprite {
	texture_handle texture;
	float2 size;
};

export std::string get_assets_path() {
	return std::format("{}\\assets", SDL_GetBasePath());
}

export struct sprite_mananger {
	SDL_Renderer* renderer = nullptr;
	std::vector<SDL_Texture*> textures;
	std::vector<std::string> texturePaths;

	sprite_mananger(SDL_Renderer* renderer)
		: renderer(renderer)
	{
	}

	std::expected<texture_handle, error> load_sprite(std::string path)
	{
		// lookup cached
		for (size_t i = 0; i < texturePaths.size(); ++i)
		{
			if (texturePaths[i] == path)
				return i;
		}

		// load
		SDL_Surface* surface = IMG_Load((get_assets_path() + "\\" + path).c_str());
		if (!surface)
			return std::unexpected(error{ .message = SDL_GetError() });

		SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
		if (!texture)
			return std::unexpected(error{ .message = SDL_GetError() });

		SDL_DestroySurface(surface);

		auto textureIdx = textures.size();
		textures.push_back(texture);
		texturePaths.push_back(path);
		return textureIdx;
	}
};

export struct engine {
	SDL_Window* window = nullptr;
	SDL_Renderer* renderer = nullptr;
	sprite_mananger* sprite_manager = nullptr;
};

export std::expected<engine, error> engine_init()
{
	if (!SDL_Init(SDL_INIT_VIDEO))
		return std::unexpected(error{ .message = SDL_GetError() });

	engine game;
	if (!SDL_CreateWindowAndRenderer("SDL issue", 288, 512, 0, &game.window, &game.renderer))
		return std::unexpected(error{ .message = SDL_GetError() });

	if(!SDL_SetRenderVSync(game.renderer, 1)) // always vsync for now to keep framerate sane
		return std::unexpected(error{ .message = SDL_GetError() });

	game.sprite_manager = new sprite_mananger(game.renderer);
	return game;
}

export void engine_deinit(engine& game)
{
	delete game.sprite_manager;
	game.sprite_manager = nullptr;
	SDL_DestroyRenderer(game.renderer);
	game.renderer = nullptr;
	SDL_DestroyWindow(game.window);
	game.window = nullptr;
	SDL_Quit();
}