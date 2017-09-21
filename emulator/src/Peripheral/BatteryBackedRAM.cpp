#include "BatteryBackedRAM.hpp"

#include "../Chipset/MMU.hpp"
#include "../Emulator.hpp"
#include "../Chipset/Chipset.hpp"
#include "../Logger.hpp"

namespace casioemu
{
	void BatteryBackedRAM::Initialise()
	{
		ram_buffer = new uint8_t[0xE00];
		// * TODO: add option to load from file
		for (size_t ix = 0; ix != 0xE00; ++ix)
			ram_buffer[ix] = 0;

		region.Setup(0x8000, 0x0E00, "BatteryBackedRAM", ram_buffer, [](MMURegion *region, size_t offset) {
			return ((uint8_t *)region->userdata)[offset - 0x8000];
		}, [](MMURegion *region, size_t offset, uint8_t data) {
			((uint8_t *)region->userdata)[offset - 0x8000] = data;
		}, emulator);
	}

	void BatteryBackedRAM::Uninitialise()
	{
		delete[] ram_buffer;
	}
}

