#include "Keyboard.hpp"

#include "../Logger.hpp"
#include "../Chipset/MMU.hpp"
#include "../Emulator.hpp"
#include "../Chipset/Chipset.hpp"

#include <lua5.3/lua.hpp>

namespace casioemu
{
	void Keyboard::Initialise()
	{
	    renderer = emulator.GetRenderer();

		interrupt_source.Setup(5, emulator);

		{
			for (auto &button : buttons)
				button.type = Button::BT_NONE;

			const char *key = "button_map";
			lua_geti(emulator.lua_state, LUA_REGISTRYINDEX, emulator.lua_model_ref);
			if (lua_getfield(emulator.lua_state, -1, key) != LUA_TTABLE)
				PANIC("key '%s' is not a table\n", key);
			lua_len(emulator.lua_state, -1);
			size_t buttons_size = lua_tointeger(emulator.lua_state, -1);
			lua_pop(emulator.lua_state, 1);

			for (size_t ix = 0; ix != buttons_size; ++ix)
			{
				if (lua_geti(emulator.lua_state, -1, ix + 1) != LUA_TTABLE)
					PANIC("key '%s'[%zu] is not a table\n", key, ix + 1);

				for (int kx = 0; kx != 5; ++kx)
					if (lua_geti(emulator.lua_state, -1 - kx, kx + 1) != LUA_TNUMBER)
						PANIC("key '%s'[%zu][%i] is not a number\n", key, ix + 1, kx + 1);

				uint8_t code = lua_tointeger(emulator.lua_state, -1);
				size_t button_ix;
				if (code == 0xFF)
				{
					button_ix = 63;
				}
				else
				{
					button_ix = ((code >> 1) & 0x38) | (code & 0x07);
					if (button_ix >= 64)
						PANIC("button index is doesn't fit 6 bits\n");
				}

				Button &button = buttons[button_ix];

				if (code == 0xFF)
					button.type = Button::BT_POWER;
				else
					button.type = Button::BT_BUTTON;
				button.rect.x = lua_tointeger(emulator.lua_state, -5);
				button.rect.y = lua_tointeger(emulator.lua_state, -4);
				button.rect.w = lua_tointeger(emulator.lua_state, -3);
				button.rect.h = lua_tointeger(emulator.lua_state, -2);
				button.ko_bit = 1 << ((code >> 4) & 0xF);
				button.ki_bit = 1 << (code & 0xF);

				lua_pop(emulator.lua_state, 6);

				button.pressed = false;
				button.stuck = false;
			}

			lua_pop(emulator.lua_state, 2);
		}

		region_ki.Setup(0xF040, 1, "Keyboard/KI", &keyboard_in, MMURegion::DefaultRead<uint8_t>, MMURegion::IgnoreWrite, emulator);

		region_ko_mask.Setup(0xF044, 2, "Keyboard/KOMask", this, [](MMURegion *region, size_t offset) {
			offset -= region->base;
			Keyboard *keyboard = ((Keyboard *)region->userdata);
			return (uint8_t)((keyboard->keyboard_out_mask & 0x03FF) >> (offset * 8));
		}, [](MMURegion *region, size_t offset, uint8_t data) {
			offset -= region->base;
			Keyboard *keyboard = ((Keyboard *)region->userdata);
			keyboard->keyboard_out_mask &= ~(((uint16_t)0xFF) << (offset * 8));
			keyboard->keyboard_out_mask |= ((uint16_t)data) << (offset * 8);
			keyboard->keyboard_out_mask &= 0x03FF;
			if (!offset)
				keyboard->RecalculateKI();
		}, emulator);

		region_ko.Setup(0xF046, 2, "Keyboard/KO", this, [](MMURegion *region, size_t offset) {
			offset -= region->base;
			Keyboard *keyboard = ((Keyboard *)region->userdata);
			return (uint8_t)((keyboard->keyboard_out & 0x03FF) >> (offset * 8));
		}, [](MMURegion *region, size_t offset, uint8_t data) {
			offset -= region->base;
			Keyboard *keyboard = ((Keyboard *)region->userdata);
			keyboard->keyboard_out &= ~(((uint16_t)0xFF) << (offset * 8));
			keyboard->keyboard_out |= ((uint16_t)data) << (offset * 8);
			keyboard->keyboard_out &= 0x03FF;
			if (!offset)
				keyboard->RecalculateKI();
		}, emulator);

		p0 = false;
		p1 = false;
		p146 = false;
		keyboard_out = 0;
		keyboard_out_mask = 0;
		RecalculateGhost();

		input_change = false;
	}

	void Keyboard::Tick()
	{
		if (input_change)
			interrupt_source.TryRaise();
	}

	void Keyboard::TickAfterInterrupts()
	{
		if (input_change && interrupt_source.Success())
			input_change = false;
	}

	void Keyboard::Frame()
	{
		SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
		for (auto &button : buttons)
		{
			if (button.type != Button::BT_NONE && button.pressed)
			{
				if (button.stuck)
					SDL_SetRenderDrawColor(renderer, 127, 0, 0, 127);
				else
					SDL_SetRenderDrawColor(renderer, 0, 0, 0, 127);
				SDL_RenderFillRect(renderer, &button.rect);
			}
		}
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
		for (auto &button : buttons)
		{
			if (button.rect.x <= x && button.rect.y <= y && button.rect.x + button.rect.w > x && button.rect.y + button.rect.h > y)
			{
				bool old_pressed = button.pressed;

				if (stick)
				{
					button.stuck = !button.stuck;
					button.pressed = button.stuck;
				}
				else
					button.pressed = true;

				if (button.pressed != old_pressed)
					RecalculateGhost();
				break;
			}
		}
	}

	void Keyboard::RecalculateGhost()
	{
		struct KOColumn
		{
			uint8_t connections;
			bool seen;
		} columns[8];

		for (size_t cx = 0; cx != 8; ++cx)
		{
			columns[cx].seen = false;
			columns[cx].connections = 0;
			for (size_t rx = 0; rx != 8; ++rx)
			{
				Button &button = buttons[cx * 8 + rx];
				if (button.type == Button::BT_BUTTON && button.pressed)
				{
					for (size_t ax = 0; ax != 8; ++ax)
					{
						Button &sibling = buttons[ax * 8 + rx];
						if (sibling.type == Button::BT_BUTTON && sibling.pressed)
							columns[cx].connections |= 1 << ax;
					}
				}
			}
		}

		for (size_t cx = 0; cx != 8; ++cx)
		{
			if (!columns[cx].seen)
			{
				uint8_t to_visit = 1 << cx;
				uint8_t ghost_mask = 1 << cx;
				columns[cx].seen = true;

				while (to_visit)
				{
					uint8_t new_to_visit = 0;
					for (size_t vx = 0; vx != 8; ++vx)
					{
						if (to_visit & (1 << vx))
						{
							for (size_t sx = 0; sx != 8; ++sx)
							{
								if (columns[vx].connections & (1 << sx) && !columns[sx].seen)
								{
									new_to_visit |= 1 << sx;
									ghost_mask |= 1 << sx;
									columns[sx].seen = true;
								}
							}
						}
					}
					to_visit = new_to_visit;
				}

				for (size_t gx = 0; gx != 8; ++gx)
					if (ghost_mask & (1 << gx))
						keyboard_ghost[gx] = ghost_mask;
			}
		}

		input_change = true;
		RecalculateKI();
	}

	void Keyboard::RecalculateKI()
	{
		uint8_t keyboard_out_ghosted = 0;
		for (size_t ix = 0; ix != 7; ++ix)
			if (keyboard_out & ~keyboard_out_mask & (1 << ix))
				keyboard_out_ghosted |= keyboard_ghost[ix];

		keyboard_in = 0xFF;
		for (auto &button : buttons)
			if (button.type == Button::BT_BUTTON && button.pressed && button.ko_bit & keyboard_out_ghosted)
				keyboard_in &= ~button.ki_bit;

		if (keyboard_out & ~keyboard_out_mask & (1 << 7) && p0)
			keyboard_in &= 0x7F;
		if (keyboard_out & ~keyboard_out_mask & (1 << 8) && p1)
			keyboard_in &= 0x7F;
		if (keyboard_out & ~keyboard_out_mask & (1 << 9) && p146)
			keyboard_in &= 0x7F;
	}

	void Keyboard::ReleaseAll()
	{
		bool had_effect = false;
		for (auto &button : buttons)
		{
			if (!button.stuck)
			{
				if (button.pressed)
				{
					button.pressed = false;
					had_effect = true;
				}
			}
		}
		if (had_effect)
			RecalculateGhost();
	}
}

