#include "Config.hpp"

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <iostream>
#include <thread>
#include <string>
#include <map>
#include <mutex>
#include <condition_variable>

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
		casioemu::Emulator emulator(argv_map, 20, 32768);

		std::mutex input_mx;
		std::condition_variable input_cv;
		bool input_processed;
		std::string console_input_str;
		Uint32 console_input_event = SDL_RegisterEvents(1);
		std::thread console_input_thread([&] {
			while (1)
			{
				std::cout << "> ";
				std::getline(std::cin, console_input_str);
				if (std::cin.fail())
				{
					casioemu::logger::Info("Console thread shutting down\n");
					break;
				}

				input_processed = false;
				SDL_Event event;
				SDL_zero(event);
				event.type = console_input_event;
				SDL_PushEvent(&event);

				std::unique_lock<std::mutex> input_lock(input_mx);
				input_cv.wait(input_lock, [&] {
					return input_processed;
				});
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
			case SDL_USEREVENT:
				if (event.type == console_input_event)
				{
					{
						std::lock_guard<std::mutex> input_lock(input_mx);
						std::lock_guard<std::mutex> access_lock(emulator.access_mx);
						emulator.ExecuteCommand(console_input_str);
						input_processed = true;
					}
					input_cv.notify_one();
				}
				break;

			case SDL_WINDOWEVENT:
				switch (event.window.event)
				{
				case SDL_WINDOWEVENT_CLOSE:
					std::lock_guard<std::mutex> access_lock(emulator.access_mx);
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

