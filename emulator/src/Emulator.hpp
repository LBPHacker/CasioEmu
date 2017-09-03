#pragma once
#include "Config.hpp"

#include <string>
#include <map>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <lua5.3/lua.hpp>

namespace casioemu
{
	class Chipset;
	class CPU;
	class MMU;

	class Emulator
	{
		SDL_Window *window;
		SDL_Surface *window_surface, *interface_image_surface;
		SDL_TimerID timer_id;
		Uint32 timer_interval;
		bool running;
		Uint32 last_frame_tick_count;
		std::string model_path;

		lua_State *lua_state;
		int lua_model_ref;

		/**
		 * The cycle manager structure. This structure is reset every time the
		 * emulator starts emulating CPU cycles and in every timer callback
		 * it's queried for the number of cycles that need to be emulated in the
		 * callback. This ensures that only as many cycles are emulated in a period
		 * of time as many would be in real life.
		 */
		struct Cycles
		{
			Cycles(Uint64 cycles_per_second);
			void Reset();
			Uint64 GetDelta();
			Uint64 ticks_at_reset, cycles_emulated, cycles_per_second, performance_frequency;
		} cycles;

		/**
		 * A smart-cast structure used to return data loaded from the model definition.
		 * `GetModelInfo` returns one of these.
		 */
		struct ModelInfo
		{
			ModelInfo(Emulator &emulator, std::string key);
			Emulator &emulator;
			std::string key;

			operator std::string();
			operator int();
		};

		/**
		 * A bunch of internally used methods for encapsulation purposes.
		 */
		void LoadModelDefition();
		void LoadInterfaceImage();
		void TimerCallback();

	public:
		Emulator(std::string model_path, Uint32 timer_interval, Uint32 cycles_per_second);
		~Emulator();

		/**
		 * A reference to the emulator chipset. This object holds all CPU, MMU, memory and
		 * peripheral state. The emulator interfaces with the chipset by issuing interrupts
		 * and rendering the screen buffer. It may also read internal state for testing purposes.
		 */
		Chipset &chipset;

		bool Running();
		void Shutdown();
		ModelInfo GetModelInfo(std::string key);
		std::string GetModelFilePath(std::string relative_path);
	};
}

