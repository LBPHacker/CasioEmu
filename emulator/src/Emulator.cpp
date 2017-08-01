#include "Emulator.hpp"

#include <iostream>
#include <fstream>
#include <sstream>

namespace casioemu
{
	Emulator::Emulator(std::string _model_path, Uint32 _timer_interval, Uint32 _cycles_per_second) : cycles(_cycles_per_second)
	{
		running = true;
		timer_interval = _timer_interval;
		model_path = _model_path;

		lua_state = luaL_newstate();
		luaL_openlibs(lua_state);

		LoadModelDefition();

		window = SDL_CreateWindow(
			std::string(GetModelInfo("model_name")).c_str(),
			SDL_WINDOWPOS_UNDEFINED,
			SDL_WINDOWPOS_UNDEFINED,
			int(GetModelInfo("interface_width")),
			int(GetModelInfo("interface_height")),
			SDL_WINDOW_SHOWN
		);
		if (!window)
			PANIC("SDL_CreateWindow failed: %s\n", SDL_GetError());

		window_surface = SDL_GetWindowSurface(window);

		LoadInterfaceImage();

		SDL_FillRect(window_surface, nullptr, SDL_MapRGB(window_surface->format, 255, 255, 255));
		SDL_BlitSurface(interface_image_surface, nullptr, window_surface, nullptr);
		SDL_UpdateWindowSurface(window);

		cycles.Reset();

		SDL_AddTimer(timer_interval, [](Uint32 delay, void *param) {
			Emulator *emulator = (Emulator *)param;
			emulator->TimerCallback();
			return emulator->timer_interval;
		}, this);
	}

	Emulator::~Emulator()
	{
	    SDL_FreeSurface(interface_image_surface);
		SDL_DestroyWindow(window);

		lua_close(lua_state);
	}

	void Emulator::LoadModelDefition()
	{
		if (luaL_loadfile(lua_state, (model_path + "/model.lua").c_str()) != LUA_OK)
			PANIC("LoadModelDefition failed: %s\n", lua_tostring(lua_state, -1));

		if (lua_pcall(lua_state, 0, 1, 0) != LUA_OK)
			PANIC("LoadModelDefition failed: %s\n", lua_tostring(lua_state, -1));

		lua_setglobal(lua_state, "model");
	}

	Emulator::ModelInfo Emulator::GetModelInfo(std::string key)
	{
		return ModelInfo(this, key);
	}

	Emulator::ModelInfo::ModelInfo(Emulator *_emulator, std::string _key)
	{
		emulator = _emulator;
		key = _key;
	}

	Emulator::ModelInfo::operator std::string()
	{
		lua_getglobal(emulator->lua_state, "model");
		lua_getfield(emulator->lua_state, -1, key.c_str());
		const char *value = lua_tostring(emulator->lua_state, -1);
		if (!value)
			value = "nil";
		std::string result(value);
		lua_pop(emulator->lua_state, 1);
		printf("%s\n", result.c_str());
		return result;
	}

	Emulator::ModelInfo::operator int()
	{
		int result;
		std::stringstream ss;
		ss << std::string(*this);
		ss >> result;
		return result;
	}

	void Emulator::LoadInterfaceImage()
	{
	    SDL_Surface *loaded_surface = IMG_Load((model_path + "/" + std::string(GetModelInfo("interface_image_path"))).c_str());
	    if (!loaded_surface)
	    	PANIC("IMG_Load failed: %s\n", IMG_GetError());

	    interface_image_surface = SDL_ConvertSurface(loaded_surface, window_surface->format, 0);
	    if (!interface_image_surface)
	    	PANIC("SDL_ConvertSurface failed: %s\n", SDL_GetError());

	    SDL_FreeSurface(loaded_surface);
	}

	void Emulator::TimerCallback()
	{
		Uint64 cycles_to_emulate = cycles.GetDelta();

		printf("Timer callback, will emulate %lu cycles\n", cycles_to_emulate);
	}

	bool Emulator::Running()
	{
		return running;
	}

	void Emulator::Shutdown()
	{
		running = false;
	}

	Emulator::Cycles::Cycles(Uint64 _cycles_per_second)
	{
		cycles_per_second = _cycles_per_second;
		performance_frequency = SDL_GetPerformanceFrequency();
	}

	void Emulator::Cycles::Reset()
	{
		ticks_at_reset = SDL_GetPerformanceCounter();
		cycles_emulated = 0;
	}

	Uint64 Emulator::Cycles::GetDelta()
	{
		Uint64 ticks_now = SDL_GetPerformanceCounter();
		Uint64 cycles_to_have_been_emulated_by_now = (double)(ticks_now - ticks_at_reset) / performance_frequency * cycles_per_second;
		Uint64 diff = cycles_to_have_been_emulated_by_now - cycles_emulated;
		cycles_emulated = cycles_to_have_been_emulated_by_now;
		return diff;
	}
}

