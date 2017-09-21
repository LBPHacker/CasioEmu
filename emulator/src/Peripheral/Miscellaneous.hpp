#pragma once
#include "../Config.hpp"

#include "Peripheral.hpp"
#include "../Chipset/MMURegion.hpp"

namespace casioemu
{
	class Miscellaneous : public Peripheral
	{
		MMURegion region_dsr, region_F048, region_F220;
		uint64_t data_F048;
		uint32_t data_F220;

		MMURegion region_F041, region_F042, region_F033, region_F034;
		uint8_t data_F041, data_F042, data_F033, data_F034;

	public:
		using Peripheral::Peripheral;

		void Initialise();
	};
}

