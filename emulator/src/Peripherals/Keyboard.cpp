#include "Keyboard.hpp"

#include "../Logger.hpp"
#include "../MMU.hpp"
#include "../Emulator.hpp"
#include "../Chipset.hpp"

#include <lua5.3/lua.hpp>

namespace casioemu
{
	void Keyboard::Initialise()
	{
		{
			const char *key = "button_map";
			lua_geti(emulator.lua_state, LUA_REGISTRYINDEX, emulator.lua_model_ref);
			if (lua_getfield(emulator.lua_state, -1, key) != LUA_TTABLE)
				PANIC("key '%s' is not a table\n", key);
			lua_len(emulator.lua_state, -1);
			buttons_size = lua_tointeger(emulator.lua_state, -1);
			lua_pop(emulator.lua_state, 1);

			buttons = new ButtonInfo[buttons_size];
			for (size_t ix = 0; ix != buttons_size; ++ix)
			{
				if (lua_geti(emulator.lua_state, -1, ix + 1) != LUA_TTABLE)
					PANIC("key '%s'[%zu] is not a table\n", key, ix + 1);

				for (int kx = 0; kx != 5; ++kx)
					if (lua_geti(emulator.lua_state, -1 - kx, kx + 1) != LUA_TNUMBER)
						PANIC("key '%s'[%zu][%i] is not a number\n", key, ix + 1, kx + 1);

				buttons[ix].x = lua_tointeger(emulator.lua_state, -5);
				buttons[ix].y = lua_tointeger(emulator.lua_state, -4);
				buttons[ix].w = lua_tointeger(emulator.lua_state, -3);
				buttons[ix].h = lua_tointeger(emulator.lua_state, -2);
				uint16_t code = lua_tointeger(emulator.lua_state, -1);
				buttons[ix].mask = code >> 8;
				buttons[ix].response = code;

				lua_pop(emulator.lua_state, 6);

				buttons[ix].pressed = false;
				buttons[ix].stuck = false;
			}

			lua_pop(emulator.lua_state, 2);
		}

		region_ki = {
			0xF040, // * base
			0x0001, // * size
			"Keyboard/KI", // * description
			this, // * userdata
			[](MMURegion *region, size_t offset) {
				return ((Keyboard *)region->userdata)->keyboard_in;
			}, // * read function
			[](MMURegion *region, size_t offset, uint8_t data) {
			} // * write function
		};
		emulator.chipset.mmu.RegisterRegion(&region_ki);

		region_ko = {
			0xF046, // * base
			0x0001, // * size
			"Keyboard/KO", // * description
			this, // * userdata
			[](MMURegion *region, size_t offset) {
				return ((Keyboard *)region->userdata)->keyboard_out;
			}, // * read function
			[](MMURegion *region, size_t offset, uint8_t data) {
				Keyboard *keyboard = ((Keyboard *)region->userdata);
				keyboard->keyboard_out = data;
				keyboard->Recalculate();
			} // * write function
		};
		emulator.chipset.mmu.RegisterRegion(&region_ko);

		keyboard_out = 0;
		Recalculate();
	}

	void Keyboard::Uninitialise()
	{
		emulator.chipset.mmu.UnregisterRegion(&region_ki);
		emulator.chipset.mmu.UnregisterRegion(&region_ko);

		delete[] buttons;
	}

	void Keyboard::Tick()
	{
	}

	void Keyboard::Frame()
	{
	}

	void Keyboard::UIEvent(SDL_Event &event)
	{
		switch (event.type)
		{
		case SDL_MOUSEBUTTONDOWN:
		case SDL_MOUSEBUTTONUP:
			switch (event.button.button)
			{
			case SDL_BUTTON_LEFT:
				if (event.button.state == SDL_PRESSED)
					PressAt(event.button.x, event.button.y, false);
				else
					ReleaseAll();
				break;

			case SDL_BUTTON_RIGHT:
				if (event.button.state == SDL_PRESSED)
					PressAt(event.button.x, event.button.y, true);
				break;
			}
			break;
		}
	}

	void Keyboard::PressAt(int x, int y, bool stick)
	{
		for (size_t ix = 0; ix != buttons_size; ++ix)
		{
			if (buttons[ix].x <= x && buttons[ix].y <= y && buttons[ix].x + buttons[ix].w > x && buttons[ix].y + buttons[ix].h > y)
			{
				if (stick)
				{
					buttons[ix].stuck = !buttons[ix].stuck;
					buttons[ix].pressed = buttons[ix].stuck;
				}
				else
					buttons[ix].pressed = true;
				Recalculate();
				break;
			}
		}
	}

	void Keyboard::Recalculate()
	{
		keyboard_in = 0;
		for (size_t ix = 0; ix != buttons_size; ++ix)
			if (buttons[ix].pressed && buttons[ix].mask == keyboard_out)
				keyboard_in |= buttons[ix].response;
	}

	void Keyboard::ReleaseAll()
	{
		for (size_t ix = 0; ix != buttons_size; ++ix)
			if (!buttons[ix].stuck)
				buttons[ix].pressed = false;
		Recalculate();
	}
}

