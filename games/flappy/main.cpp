#include <format>
#include <SDL3/SDL.h>

int main(int argc, char* argv[]) {
	(void)argc;
	(void)argv;

	if (!SDL_Init(SDL_INIT_VIDEO)) {
		SDL_Log("SDL_Init failed (%s)", SDL_GetError());
		return 1;
	}

	SDL_Window* window = NULL;
	SDL_Renderer* renderer = NULL;
	if (!SDL_CreateWindowAndRenderer("SDL issue", 800, 600, 0, &window, &renderer)) {
		SDL_Log("SDL_CreateWindowAndRenderer failed (%s)", SDL_GetError());
		SDL_Quit();
		return 1;
	}

	SDL_Surface* surface = SDL_LoadBMP(std::format("{}\\data\\audiofile.bmp", SDL_GetBasePath()).c_str());
	if (!surface) {
		SDL_Log("Couldn't load bitmap: %s", SDL_GetError());
		return 1;
	}

	SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
	if (!texture) {
		SDL_Log("Couldn't create static texture: %s", SDL_GetError());
		return SDL_APP_FAILURE;
	}

	SDL_DestroySurface(surface);

	bool foo = false;
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
				else
				{
					foo = true;
				}
			}
		}
		if (finished)
			break;

		SDL_SetRenderDrawColor(renderer, 80, 80, 80, SDL_ALPHA_OPAQUE);
		SDL_RenderClear(renderer);

		SDL_SetRenderDrawColor(renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);
		SDL_RenderDebugText(renderer, 5, 5, foo ? "Key pressed" : "Hello world!");

		SDL_FRect dst_rect;
		dst_rect.x = 32;
		dst_rect.y = 32;
		dst_rect.w = 64;
		dst_rect.h = 64;
		SDL_RenderTexture(renderer, texture, NULL, &dst_rect);

		SDL_RenderPresent(renderer);
	}

	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);

	SDL_Quit();
	return 0;
}