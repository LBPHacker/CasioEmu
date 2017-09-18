#include "BatteryBackedRAM.hpp"

#include "../MMU.hpp"
#include "../Emulator.hpp"
#include "../Chipset.hpp"
#include "../Logger.hpp"

namespace casioemu
{
	void BatteryBackedRAM::Initialise()
	{
		ram_buffer = new uint8_t[0x0E00];

		region = {
			0x8000, // * base
			0x0E00, // * size
			"BatteryBackedRAM", // * description
			ram_buffer, // * userdata
			[](MMURegion *region, size_t offset) {
				return ((uint8_t *)region->userdata)[offset - 0x8000];
			}, // * read function
			[](MMURegion *region, size_t offset, uint8_t data) {
				((uint8_t *)region->userdata)[offset - 0x8000] = data;
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

	void BatteryBackedRAM::Frame()
	{
	}

	void BatteryBackedRAM::UIEvent(SDL_Event &event)
	{
	}
}

