#pragma once

#include <string>
#include <map>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include "Config.hpp"

namespace casioemu
{
	class Emulator
	{
		SDL_Window *window;
		SDL_Surface *window_surface, *interface_image_surface;
		Uint32 timer_interval;
		bool running;
		Uint32 last_frame_tick_count;
		std::string model_path;

		struct Config
		{
			Config(std::string model_path);
			std::string interface_image_path, rom_path, model_name;
			int interface_width, interface_height;
		} config;

		struct Cycles
		{
			Cycles(Uint64 cycles_per_second);
			void Reset();
			Uint64 GetDelta();
			Uint64 ticks_at_reset, cycles_emulated, cycles_per_second, performance_frequency;
		} cycles;

		void LoadInterfaceImage();
		void TimerCallback();

	public:
		Emulator(std::string model_path, Uint32 timer_interval, Uint32 cycles_per_second);
		~Emulator();
		bool Running();
		void Shutdown();
	};
}
