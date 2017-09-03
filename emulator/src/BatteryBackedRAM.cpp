#include "BatteryBackedRAM.hpp"

#include "MMU.hpp"
#include "Emulator.hpp"
#include "Chipset.hpp"

namespace casioemu
{
	void BatteryBackedRAM::Initialise()
	{
		ram_buffer = new uint8_t[PL_BATTERYBACKEDRAM_SIZE];

		region = {
			PL_BATTERYBACKEDRAM_BASE, // * base
			PL_BATTERYBACKEDRAM_SIZE, // * size
			"BatteryBackedRAM", // * description
			ram_buffer, // * userdata
			[](MMURegion *region, size_t offset, size_t length) {
				offset -= PL_BATTERYBACKEDRAM_BASE;
				uint64_t result = 0;
				for (size_t ix = 0; ix != length; ++ix)
					result |= uint64_t(((uint8_t *)region->userdata)[offset + ix]) << (8 * ix);
				return result;
			}, // * read function
			[](MMURegion *region, size_t offset, size_t length, uint64_t data) {
				offset -= PL_BATTERYBACKEDRAM_BASE;
				for (size_t ix = 0; ix != length; ++ix)
					((uint8_t *)region->userdata)[offset + ix] = data >> (8 * ix);
			} // * write function
		};

		emulator.chipset.mmu.RegisterRegion(&region);
	}

	void BatteryBackedRAM::Uninitialise()
	{
		emulator.chipset.mmu.UnregisterRegion(&region);

		delete[] ram_buffer;
	}

	void BatteryBackedRAM::Tick()
	{
	}
}

