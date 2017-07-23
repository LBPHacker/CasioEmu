#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include "Config.hpp"

#include "Emulator.hpp"




int main(int argc, char *argv[])
{
	if (argc < 2)
	{
		printf("No model path supplied\n");
		exit(2);
	}

	if (SDL_Init(SDL_INIT_VIDEO & SDL_INIT_TIMER) < 0)
		Panic("SDL_Init failed: %s\n", SDL_GetError());

	int imgFlags = IMG_INIT_PNG;
	if (!(IMG_Init(imgFlags) & imgFlags))
		Panic("IMG_Init failed: %s\n", IMG_GetError());

	casioemu::Emulator emulator(argv[1]);
	while (emulator.Running())
	{
		SDL_Event event;
		if (!SDL_WaitEvent(&event))
			Panic("SDL_WaitEvent failed: %s\n", SDL_GetError());

		switch (event.type)
		{
		// case SDL_USEREVENT:
		// 	printf("Timed out\n");
		// 	running = false;
		// 	break;
		case SDL_WINDOWEVENT:
			switch (event.window.event)
			{
			case SDL_WINDOWEVENT_CLOSE:
				emulator.Shutdown();
				break;
			}
			break;
		}
	}

	SDL_Quit();
	return 0;
}

