#include "Peripheral.hpp"

namespace casioemu
{
	Peripheral::Peripheral(Emulator &_emulator) : emulator(_emulator)
	{
	}

	Peripheral::~Peripheral()
	{
	}

	void Peripheral::Initialise()
	{
	}

	void Peripheral::Uninitialise()
	{
	}

	void Peripheral::Tick()
	{
	}

	void Peripheral::TickAfterInterrupts()
	{
	}

	void Peripheral::Frame()
	{
	}

	void Peripheral::UIEvent(SDL_Event &event)
	{
	}
}

