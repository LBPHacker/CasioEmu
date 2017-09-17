#pragma once
#include "../Config.hpp"

#include "../Peripheral.hpp"
#include "../MMURegion.hpp"

namespace casioemu
{
	class Screen : public Peripheral
	{
		MMURegion region;
		uint8_t *ram_buffer;

	public:
		using Peripheral::Peripheral;

		void Initialise();
		void Uninitialise();
		void Tick();
	};
}

