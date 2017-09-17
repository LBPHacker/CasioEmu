#include "Screen.hpp"

#include "../Logger.hpp"
#include "../MMU.hpp"
#include "../Emulator.hpp"
#include "../Chipset.hpp"

namespace casioemu
{
	void Screen::Initialise()
	{
		ram_buffer = new uint8_t[0x0200];

		region = {
			0xF800, // * base
			0x0200, // * size
			"Screen", // * description
			ram_buffer, // * userdata
			[](MMURegion *region, size_t offset) {
				return ((uint8_t *)region->userdata)[offset - 0xF800];
			}, // * read function
			[](MMURegion *region, size_t offset, uint8_t data) {
				((uint8_t *)region->userdata)[offset - 0xF800] = data;
			} // * write function
		};
		emulator.chipset.mmu.RegisterRegion(&region);
	}

	void Screen::Uninitialise()
	{
		emulator.chipset.mmu.UnregisterRegion(&region);

		delete[] ram_buffer;
	}

	void Screen::Tick()
	{
	}

	void Screen::Frame()
	{
	}
}
