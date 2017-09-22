#pragma once
#include "../Config.hpp"

#include "Peripheral.hpp"
#include "../Chipset/MMURegion.hpp"

namespace casioemu
{
	class BatteryBackedRAM : public Peripheral
	{
		MMURegion region;
		uint8_t *ram_buffer;
		bool ram_file_requested;

	public:
		using Peripheral::Peripheral;

		void Initialise();
		void Uninitialise();
		void SaveImage();
		void LoadImage();
	};
}

