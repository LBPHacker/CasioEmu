#include "Config.hpp"

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <iostream>
#include <thread>
#include <string>
#include <map>

#include "Emulator.hpp"
#include "Logger.hpp"

int main(int argc, char *argv[])
{
	std::map<std::string, std::string> argv_map;
	for (int ix = 1; ix != argc; ++ix)
	{
		std::string key, value;
		char *eq_pos = strchr(argv[ix], '=');
		if (eq_pos)
		{
			key = std::string(argv[ix], eq_pos);
			value = eq_pos + 1;
		}
		else
		{
			key = "model";
			value = argv[ix];
		}

		if (argv_map.find(key) == argv_map.end())
			argv_map[key] = value;
		else
			casioemu::logger::Info("[argv] #%i: key '%s' already set\n", ix, key.c_str());
	}

	if (argv_map.find("model") == argv_map.end())
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
		casioemu::Emulator emulator(argv_map["model"], 20, 32768);
		if (argv_map.find("paused") != argv_map.end())
			emulator.SetPaused(true);

		std::thread console_input_thread([&emulator]() {
			std::string console_input_str;
			while (1)
			{
				std::cout << "> ";
				std::getline(std::cin, console_input_str);
				std::lock_guard<std::mutex> access_guard(emulator.access_lock);
				emulator.ExecuteCommand(console_input_str);
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

