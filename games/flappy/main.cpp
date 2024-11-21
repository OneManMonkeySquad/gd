#include <SDL3/SDL.h>
/*
 * SDL3/SDL_main.h is explicitly not included such that a terminal window would appear on Windows.
 */

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

		SDL_RenderPresent(renderer);
	}

	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);

	SDL_Quit();
	return 0;
}