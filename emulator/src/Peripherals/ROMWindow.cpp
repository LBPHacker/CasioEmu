#include "ROMWindow.hpp"

#include "../Logger.hpp"
#include "../MMU.hpp"
#include "../Emulator.hpp"
#include "../Chipset.hpp"

namespace casioemu
{
	void ROMWindow::Initialise()
	{
		region = {
			PL_ROMWINDOW_BASE, // * base
			PL_ROMWINDOW_SIZE, // * size
			"ROMWindow", // * description
			&emulator, // * userdata
			[](MMURegion *region, size_t offset) {
				uint16_t rom_data = ((Emulator *)(region->userdata))->chipset.mmu.ReadCode(offset & ~1);
				if (offset & 1)
					rom_data >>= 8;
				return (uint8_t)rom_data;
			}, // * read function
			[](MMURegion *region, size_t offset, uint8_t data) {
				logger::Info("ROMWindow::[region write lamda]: Attempt to write %02hhX to %06zX\n", data, offset);
			} // * write function
		};

		emulator.chipset.mmu.RegisterRegion(&region);
	}

	void ROMWindow::Uninitialise()
	{
		emulator.chipset.mmu.UnregisterRegion(&region);
	}

	void ROMWindow::Tick()
	{
	}
}

