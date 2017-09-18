#include "Emulator.hpp"

#include "Chipset/Chipset.hpp"
#include "Logger.hpp"
#include "Data/EventCode.hpp"

#include <iostream>
#include <fstream>
#include <sstream>

namespace casioemu
{
	Emulator::Emulator(std::map<std::string, std::string> &_argv_map, Uint32 _timer_interval, Uint32 _cycles_per_second, bool _paused) : paused(_paused), argv_map(_argv_map), cycles(_cycles_per_second), chipset(*new Chipset(*this))
	{
		std::lock_guard<std::mutex> access_lock(access_mx);
		running = true;
		timer_interval = _timer_interval;
		model_path = argv_map["model"];

		lua_state = luaL_newstate();
		luaL_openlibs(lua_state);

		SetupLuaAPI();
		LoadModelDefition();

		interface_background = GetModelInfo("rsd_interface");

		window = SDL_CreateWindow(
			std::string(GetModelInfo("model_name")).c_str(),
			SDL_WINDOWPOS_UNDEFINED,
			SDL_WINDOWPOS_UNDEFINED,
			interface_background.src.w,
			interface_background.src.h,
			SDL_WINDOW_SHOWN
		);
		if (!window)
			PANIC("SDL_CreateWindow failed: %s\n", SDL_GetError());
		renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
		if (!renderer)
			PANIC("SDL_CreateRenderer failed: %s\n", SDL_GetError());

		SDL_Surface *loaded_surface = IMG_Load(GetModelFilePath(GetModelInfo("interface_image_path")).c_str());
		if (!loaded_surface)
			PANIC("IMG_Load failed: %s\n", IMG_GetError());
		interface_texture = SDL_CreateTextureFromSurface(renderer, loaded_surface);
		SDL_FreeSurface(loaded_surface);

		SetupInternals();
		cycles.Reset();

		timer_id = SDL_AddTimer(timer_interval, [](Uint32 delay, void *param) {
			Emulator *emulator = (Emulator *)param;
			emulator->TimerCallback();
			return emulator->timer_interval;
		}, this);

		RunStartupScript();

		chipset.Reset();

		if (argv_map.find("paused") != argv_map.end())
			SetPaused(true);
	}

	Emulator::~Emulator()
	{
		std::lock_guard<std::mutex> access_lock(access_mx);
		SDL_RemoveTimer(timer_id);

		SDL_DestroyTexture(interface_texture);
		SDL_DestroyRenderer(renderer);
		SDL_DestroyWindow(window);

		luaL_unref(lua_state, LUA_REGISTRYINDEX, lua_model_ref);
		lua_close(lua_state);
		delete &chipset;
	}

	void Emulator::UIEvent(SDL_Event &event)
	{
		chipset.UIEvent(event);
	}

	void Emulator::RunStartupScript()
	{
		if (argv_map.find("script") == argv_map.end())
			return;

		if (luaL_loadfile(lua_state, argv_map["script"].c_str()) != LUA_OK)
		{
			logger::Info("%s\n", lua_tostring(lua_state, -1));
			lua_pop(lua_state, 1);
			return;
		}

		if (lua_pcall(lua_state, 0, 1, 0) != LUA_OK)
		{
			logger::Info("%s\n", lua_tostring(lua_state, -1));
			lua_pop(lua_state, 1);
			return;
		}
	}

	void Emulator::SetupLuaAPI()
	{
		*(Emulator **)lua_newuserdata(lua_state, sizeof(Emulator *)) = this;
		lua_newtable(lua_state);
		lua_newtable(lua_state);
		lua_pushcfunction(lua_state, [](lua_State *lua_state) {
			Emulator *emu = *(Emulator **)lua_topointer(lua_state, 1);
			emu->Tick();
			return 0;
		});
		lua_setfield(lua_state, -2, "tick");
		lua_pushcfunction(lua_state, [](lua_State *lua_state) {
			Emulator *emu = *(Emulator **)lua_topointer(lua_state, 1);
			emu->Shutdown();
			return 0;
		});
		lua_setfield(lua_state, -2, "shutdown");
		lua_pushcfunction(lua_state, [](lua_State *lua_state) {
			Emulator *emu = *(Emulator **)lua_topointer(lua_state, 1);
			emu->SetPaused(lua_toboolean(lua_state, 2));
			return 0;
		});
		lua_setfield(lua_state, -2, "set_paused");
		lua_model_ref = LUA_REFNIL;
		lua_pushcfunction(lua_state, [](lua_State *lua_state) {
			Emulator *emu = *(Emulator **)lua_topointer(lua_state, 1);
			if (emu->lua_model_ref != LUA_REFNIL)
				PANIC("emu.model invoked twice\n");
			emu->lua_model_ref = luaL_ref(lua_state, LUA_REGISTRYINDEX);
			return 0;
		});
		lua_setfield(lua_state, -2, "model");
		lua_pre_tick_ref = LUA_REFNIL;
		lua_pushcfunction(lua_state, [](lua_State *lua_state) {
			Emulator *emu = *(Emulator **)lua_topointer(lua_state, 1);
			luaL_unref(lua_state, LUA_REGISTRYINDEX, emu->lua_pre_tick_ref);
			emu->lua_pre_tick_ref = luaL_ref(lua_state, LUA_REGISTRYINDEX);
			return 0;
		});
		lua_setfield(lua_state, -2, "pre_tick");
		lua_post_tick_ref = LUA_REFNIL;
		lua_pushcfunction(lua_state, [](lua_State *lua_state) {
			Emulator *emu = *(Emulator **)lua_topointer(lua_state, 1);
			luaL_unref(lua_state, LUA_REGISTRYINDEX, emu->lua_post_tick_ref);
			emu->lua_post_tick_ref = luaL_ref(lua_state, LUA_REGISTRYINDEX);
			return 0;
		});
		lua_setfield(lua_state, -2, "post_tick");
		lua_setfield(lua_state, -2, "__index");
		lua_pushcfunction(lua_state, [](lua_State *lua_state) {
			return 0;
		});
		lua_setfield(lua_state, -2, "__newindex");
		lua_setmetatable(lua_state, -2);
		lua_setglobal(lua_state, "emu");
	}

	void Emulator::SetupInternals()
	{
		chipset.SetupInternals();
	}

	void Emulator::LoadModelDefition()
	{
		if (luaL_loadfile(lua_state, (model_path + "/model.lua").c_str()) != LUA_OK)
			PANIC("LoadModelDefition failed: %s\n", lua_tostring(lua_state, -1));

		if (lua_pcall(lua_state, 0, 0, 0) != LUA_OK)
			PANIC("LoadModelDefition failed: %s\n", lua_tostring(lua_state, -1));

		if (lua_model_ref == LUA_REFNIL)
			PANIC("LoadModelDefition failed: model failed to call emu.model\n");
	}

	std::string Emulator::GetModelFilePath(std::string relative_path)
	{
		return model_path + "/" + relative_path;
	}

	void Emulator::TimerCallback()
	{
		std::lock_guard<std::mutex> access_lock(access_mx);
		Uint64 cycles_to_emulate = cycles.GetDelta();

		for (Uint64 ix = 0; ix != cycles_to_emulate; ++ix)
			if (!paused)
				Tick();

		SDL_Event event;
		SDL_zero(event);
		event.type = SDL_USEREVENT;
		event.user.code = CE_FRAME_REQUEST;
		SDL_PushEvent(&event);
	}

	void Emulator::Frame()
	{
		SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
		SDL_RenderClear(renderer);
		SDL_SetTextureColorMod(interface_texture, 255, 255, 255);
		SDL_SetTextureAlphaMod(interface_texture, 255);
		SDL_RenderCopy(renderer, interface_texture, &interface_background.src, &interface_background.dest);
		chipset.Frame();
		SDL_RenderPresent(renderer);
	}

	void Emulator::Tick()
	{
		if (lua_pre_tick_ref != LUA_REFNIL)
		{
			lua_geti(lua_state, LUA_REGISTRYINDEX, lua_pre_tick_ref);
			if (lua_pcall(lua_state, 0, 0, 0) != LUA_OK)
			{
				logger::Info("pre-tick hook failed: %s\n", lua_tostring(lua_state, -1));
				lua_pop(lua_state, 1);
				luaL_unref(lua_state, LUA_REGISTRYINDEX, lua_pre_tick_ref);
				lua_pre_tick_ref = LUA_REFNIL;
				logger::Info("  pre-tick hook unregistered\n", lua_tostring(lua_state, -1));
			}
		}

		chipset.Tick();

		if (lua_post_tick_ref != LUA_REFNIL)
		{
			lua_geti(lua_state, LUA_REGISTRYINDEX, lua_post_tick_ref);
			if (lua_pcall(lua_state, 0, 0, 0) != LUA_OK)
			{
				logger::Info("post-tick hook failed: %s\n", lua_tostring(lua_state, -1));
				lua_pop(lua_state, 1);
				luaL_unref(lua_state, LUA_REGISTRYINDEX, lua_post_tick_ref);
				lua_post_tick_ref = LUA_REFNIL;
				logger::Info("  post-tick hook unregistered\n", lua_tostring(lua_state, -1));
			}
		}
	}

	bool Emulator::Running()
	{
		return running;
	}

	bool Emulator::GetPaused()
	{
		return paused;
	}

	void Emulator::Shutdown()
	{
		running = false;
	}

	void Emulator::ExecuteCommand(std::string command)
	{
		const char *ugly_string_data_ptr = command.c_str();
		if (lua_load(lua_state, [](lua_State *lua_state, void *data, size_t *size) {
			char **ugly_string_data_ptr_ptr = (char **)data;
			if (!*ugly_string_data_ptr_ptr)
				return (const char *)nullptr;
			const char *result = *ugly_string_data_ptr_ptr;
			*size = strlen(result);
			*ugly_string_data_ptr_ptr = nullptr;
			return result;
		}, &ugly_string_data_ptr, "stdin", "t") != LUA_OK)
		{
			logger::Info("%s\n", lua_tostring(lua_state, -1));
			lua_pop(lua_state, 1);
			return;
		}

		if (lua_pcall(lua_state, 0, 0, 0) != LUA_OK)
		{
			logger::Info("%s\n", lua_tostring(lua_state, -1));
			lua_pop(lua_state, 1);
			return;
		}
	}

	void Emulator::SetPaused(bool _paused)
	{
		paused = _paused;
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

    SDL_Renderer *Emulator::GetRenderer()
    {
    	return renderer;
    }

    SDL_Texture *Emulator::GetInterfaceTexture()
    {
    	return interface_texture;
    }
}

