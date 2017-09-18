#pragma once
#include "../Config.hpp"

#include <SDL2/SDL.h>

namespace casioemu
{
	class Emulator;

	class Peripheral
	{
	protected:
		Emulator &emulator;

	public:
		Peripheral(Emulator &emulator);
		virtual void Initialise() = 0;
		virtual void Uninitialise() = 0;
		virtual void Tick() = 0;
		virtual void Frame() = 0;
		virtual void UIEvent(SDL_Event &event) = 0;
		virtual ~Peripheral();
	};
}

