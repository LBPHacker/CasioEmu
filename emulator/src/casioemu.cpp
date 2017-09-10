#include "Config.hpp"

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <iostream>
#include <thread>
#include <string>

#include "Emulator.hpp"
#include "Logger.hpp"

int main(int argc, char *argv[])
{
	if (argc < 2)
	{
		printf("No model path supplied\n");
		exit(2);
	}

	int sdlFlags = SDL_INIT_VIDEO & SDL_INIT_TIMER;
	if (SDL_Init(sdlFlags) != sdlFlags)
		PANIC("SDL_Init failed: %s\n", SDL_GetError());

	int imgFlags = IMG_INIT_PNG;
	if (IMG_Init(imgFlags) != imgFlags)
		PANIC("IMG_Init failed: %s\n", IMG_GetError());

	{
		casioemu::Emulator emulator(argv[1], 20, 32768, true);

		std::thread console_input_thread([&emulator]() {
			std::string console_input_str;
			while (1)
			{
				std::cout << "> ";
				std::getline(std::cin, console_input_str);
				std::lock_guard<std::mutex> access_guard(emulator.access_lock);
				if (!emulator.ExecuteCommand(console_input_str))
					std::cout << ">";
			}
		});
		console_input_thread.detach();

		while (emulator.Running())
		{
			SDL_Event event;
			if (!SDL_WaitEvent(&event))
				PANIC("SDL_WaitEvent failed: %s\n", SDL_GetError());

			switch (event.type)
			{
			case SDL_WINDOWEVENT:
				switch (event.window.event)
				{
				case SDL_WINDOWEVENT_CLOSE:
					std::lock_guard<std::mutex> access_guard(emulator.access_lock);
					emulator.Shutdown();
					break;
				}
				break;
			}
		}
	}

	IMG_Quit();
	SDL_Quit();

	return 0;
}

