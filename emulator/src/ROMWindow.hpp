#pragma once
#include "Config.hpp"

#include "Peripheral.hpp"
#include "MMURegion.hpp"

namespace casioemu
{
	class ROMWindow : public Peripheral
	{
		MMURegion region;

	public:
		using Peripheral::Peripheral;

		void Initialise();
		void Uninitialise();
		void Tick();
	};
}

