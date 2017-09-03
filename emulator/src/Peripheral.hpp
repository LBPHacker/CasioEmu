#pragma once
#include "Config.hpp"

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
		virtual ~Peripheral();
	};
}

