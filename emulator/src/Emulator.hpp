#pragma once
#include "Config.hpp"

#include <string>
#include <map>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <lua5.3/lua.hpp>
#include <mutex>

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
		bool running, paused;
		Uint32 last_frame_tick_count;
		std::string model_path;
		std::map<std::string, std::string> &argv_map;

		lua_State *lua_state;
		int lua_model_ref, lua_pre_tick_ref, lua_post_tick_ref;

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
		void SetupLuaAPI();
		void SetupInternals();
		void RunStartupScript();

	public:
		Emulator(std::map<std::string, std::string> &argv_map, Uint32 timer_interval, Uint32 cycles_per_second, bool paused = false);
		~Emulator();

		std::mutex access_mx;

		/**
		 * A reference to the emulator chipset. This object holds all CPU, MMU, memory and
		 * peripheral state. The emulator interfaces with the chipset by issuing interrupts
		 * and rendering the screen buffer. It may also read internal state for testing purposes.
		 */
		Chipset &chipset;

		bool Running();
		void Shutdown();
		void Tick();
		void ExecuteCommand(std::string command);
		bool GetPaused();
		void SetPaused(bool paused);
		ModelInfo GetModelInfo(std::string key);
		std::string GetModelFilePath(std::string relative_path);

		friend class CPU;
		friend class MMU;
	};
}

