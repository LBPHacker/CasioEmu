#pragma once
#include "../Config.hpp"

#include "Peripheral.hpp"
#include "../Chipset/MMURegion.hpp"

namespace casioemu
{
	class Keyboard : public Peripheral
	{
		MMURegion region_ko_mask, region_ko, region_ki;
		uint16_t keyboard_out, keyboard_out_mask;
		uint8_t keyboard_in, keyboard_ghost[8];

	    SDL_Renderer *renderer;

		struct Button
		{
			enum ButtonType
			{
				BT_NONE,
				BT_BUTTON,
				BT_POWER
			} type;
			SDL_Rect rect;
			uint8_t ko_bit, ki_bit;
			bool pressed, stuck;
		} buttons[64];

		bool p0, p1, p146;

	public:
		using Peripheral::Peripheral;

		void Initialise();
		void Uninitialise();
		void Tick();
		void Frame();
		void UIEvent(SDL_Event &event);
		void PressAt(int x, int y, bool stick);
		void ReleaseAll();
		void RecalculateKI();
		void RecalculateGhost();
	};
}

