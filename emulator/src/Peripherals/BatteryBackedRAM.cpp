#include "BatteryBackedRAM.hpp"

#include "../MMU.hpp"
#include "../Emulator.hpp"
#include "../Chipset.hpp"
#include "../Logger.hpp"

#define PL_BATTERYBACKEDRAM_BASE 0x8000
#define PL_BATTERYBACKEDRAM_SIZE 0x0E00

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
			[](MMURegion *region, size_t offset) {
				return ((uint8_t *)region->userdata)[offset - PL_BATTERYBACKEDRAM_BASE];
			}, // * read function
			[](MMURegion *region, size_t offset, uint8_t data) {
				((uint8_t *)region->userdata)[offset - PL_BATTERYBACKEDRAM_BASE] = data;
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

