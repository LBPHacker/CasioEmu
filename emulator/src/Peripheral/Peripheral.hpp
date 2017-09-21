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
		virtual void Initialise();
		virtual void Uninitialise();
		virtual void Tick();
		virtual void TickAfterInterrupts();
		virtual void Frame();
		virtual void UIEvent(SDL_Event &event);
		virtual ~Peripheral();
	};
}

