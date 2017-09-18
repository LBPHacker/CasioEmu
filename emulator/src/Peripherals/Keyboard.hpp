#pragma once
#include "../Config.hpp"

#include "../Peripheral.hpp"
#include "../MMURegion.hpp"

namespace casioemu
{
	class Keyboard : public Peripheral
	{
		MMURegion region_ko, region_ki;
		uint8_t keyboard_out, keyboard_in;

		struct ButtonInfo
		{
			int x, y, w, h;
			uint8_t mask, response;
			bool pressed, stuck;
		} *buttons;
		size_t buttons_size;

	public:
		using Peripheral::Peripheral;

		void Initialise();
		void Uninitialise();
		void Tick();
		void Frame();
		void UIEvent(SDL_Event &event);
		void PressAt(int x, int y, bool stick);
		void ReleaseAll();
		void Recalculate();
	};
}

