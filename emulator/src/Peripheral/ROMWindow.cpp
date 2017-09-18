#include "ROMWindow.hpp"

#include "../Logger.hpp"
#include "../Chipset/MMU.hpp"
#include "../Emulator.hpp"
#include "../Chipset/Chipset.hpp"

namespace casioemu
{
	void ROMWindow::Initialise()
	{
		region_0 = {
			0x00000, // * base
			0x08000, // * size
			"ROM/Segment0", // * description
			&emulator, // * userdata
			[](MMURegion *region, size_t offset) {
				return ((Emulator *)(region->userdata))->chipset.rom_data[offset];
			}, // * read function
			[](MMURegion *region, size_t offset, uint8_t data) {
				logger::Info("ROM::[region write lambda]: attempt to write %02hhX to %06zX\n", data, offset);
			} // * write function
		};
		emulator.chipset.mmu.RegisterRegion(&region_0);

		region_1 = {
			0x10000, // * base
			0x10000, // * size
			"ROM/Segment1", // * description
			&emulator, // * userdata
			[](MMURegion *region, size_t offset) {
				return ((Emulator *)(region->userdata))->chipset.rom_data[offset];
			}, // * read function
			[](MMURegion *region, size_t offset, uint8_t data) {
				logger::Info("ROM::[region write lambda]: attempt to write %02hhX to %06zX\n", data, offset);
			} // * write function
		};
		emulator.chipset.mmu.RegisterRegion(&region_1);

		region_8 = {
			0x80000, // * base
			0x10000, // * size
			"ROM/Segment8", // * description
			&emulator, // * userdata
			[](MMURegion *region, size_t offset) {
				return ((Emulator *)(region->userdata))->chipset.rom_data[offset - 0x80000];
			}, // * read function
			[](MMURegion *region, size_t offset, uint8_t data) {
				logger::Info("ROM::[region write lambda]: attempt to write %02hhX to %06zX\n", data, offset);
			} // * write function
		};
		emulator.chipset.mmu.RegisterRegion(&region_8);
	}

	void ROMWindow::Uninitialise()
	{
		emulator.chipset.mmu.UnregisterRegion(&region_8);
		emulator.chipset.mmu.UnregisterRegion(&region_1);
		emulator.chipset.mmu.UnregisterRegion(&region_0);
	}

	void ROMWindow::Tick()
	{
	}

	void ROMWindow::Frame()
	{
	}

	void ROMWindow::UIEvent(SDL_Event &event)
	{
	}
}

