#pragma once
#include "Config.hpp"

#include <string>
#include <map>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <lua5.3/lua.hpp>
#include <mutex>

#include "Data/ModelInfo.hpp"
#include "Data/SpriteInfo.hpp"

namespace casioemu
{
	class Chipset;
	class CPU;
	class MMU;

	class Emulator
	{
		SDL_Window *window;
		SDL_Renderer *renderer;
		SDL_Texture *interface_texture;
		SDL_TimerID timer_id;
		unsigned int timer_interval;
		bool running, paused;
		unsigned int last_frame_tick_count;
		std::string model_path;
		bool pause_on_mem_error;

		SpriteInfo interface_background;

		/**
		 * A bunch of internally used methods for encapsulation purposes.
		 */
		void LoadModelDefition();
		void TimerCallback();
		void SetupLuaAPI();
		void SetupInternals();
		void RunStartupScript();

	public:
		Emulator(std::map<std::string, std::string> &argv_map, unsigned int timer_interval, unsigned int cycles_per_second, bool paused = false);
		~Emulator();

		std::mutex access_mx;
		lua_State *lua_state;
		int lua_model_ref, lua_pre_tick_ref, lua_post_tick_ref;
		std::map<std::string, std::string> &argv_map;

	private:
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

	public:
		/**
		 * A reference to the emulator chipset. This object holds all CPU, MMU, memory and
		 * peripheral state. The emulator interfaces with the chipset by issuing interrupts
		 * and rendering the screen buffer. It may also read internal state for testing purposes.
		 */
		Chipset &chipset;

		bool Running();
		void HandleMemoryError();
		void Shutdown();
		void Tick();
		void Frame();
		void ExecuteCommand(std::string command);
		unsigned int GetCyclesPerSecond();
		bool GetPaused();
		void SetPaused(bool paused);
		void UIEvent(SDL_Event &event);
		SDL_Renderer *GetRenderer();
		SDL_Texture *GetInterfaceTexture();
		ModelInfo GetModelInfo(std::string key);
		std::string GetModelFilePath(std::string relative_path);

		friend class ModelInfo;
		friend class CPU;
		friend class MMU;
	};
}

