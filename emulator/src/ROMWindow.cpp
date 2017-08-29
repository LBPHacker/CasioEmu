#include "ROMWindow.hpp"

#include "Logger.hpp"
#include "MMU.hpp"
#include "Emulator.hpp"
#include "Chipset.hpp"

namespace casioemu
{
	void ROMWindow::Initialise()
	{
		region = {
			PL_ROMWINDOW_BASE, // * base
			PL_ROMWINDOW_SIZE, // * size
			"ROMWindow", // * description
			&emulator, // * userdata
			[](const MMURegion &region, size_t offset, size_t length) {
				return ((Emulator *)(region.userdata))->chipset.mmu.ReadCode(offset, length);
			}, // * read function
			[](const MMURegion &region, size_t offset, size_t length, uint64_t data) {
				logger::Info("ROMWindow::[region write lamda]: Attempt to write %016llX to %06zX\n", data, offset);
			} // * write function
		};

		emulator.chipset.mmu.RegisterRegion(region);
	}

	void ROMWindow::Uninitialise()
	{
		emulator.chipset.mmu.UnregisterRegion(region);
	}
}

