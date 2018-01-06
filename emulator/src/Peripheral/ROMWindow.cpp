#include "ROMWindow.hpp"

#include "../Logger.hpp"
#include "../Chipset/MMU.hpp"
#include "../Emulator.hpp"
#include "../Chipset/Chipset.hpp"

namespace casioemu
{
	void ROMWindow::Initialise()
	{
		region_0.Setup(0x00000, 0x08000, "ROM/Segment0", this, [](MMURegion *region, size_t offset) {
			return ((ROMWindow *)(region->userdata))->emulator.chipset.rom_data[offset];
		}, [](MMURegion *region, size_t offset, uint8_t data) {
			ROMWindow *rom_window = (ROMWindow *)(region->userdata);
			if (rom_window->strict_memory)
			{
				logger::Info("ROM::[region write lambda]: attempt to write %02hhX to %06zX\n", data, offset);
				rom_window->emulator.HandleMemoryError();
			}
		}, emulator);

		region_1.Setup(0x10000, 0x10000, "ROM/Segment1", this, [](MMURegion *region, size_t offset) {
			return ((ROMWindow *)(region->userdata))->emulator.chipset.rom_data[offset];
		}, [](MMURegion *region, size_t offset, uint8_t data) {
			ROMWindow *rom_window = (ROMWindow *)(region->userdata);
			if (rom_window->strict_memory)
			{
				logger::Info("ROM::[region write lambda]: attempt to write %02hhX to %06zX\n", data, offset);
				rom_window->emulator.HandleMemoryError();
			}
		}, emulator);

		region_8.Setup(0x80000, 0x10000, "ROM/Segment8", this, [](MMURegion *region, size_t offset) {
			return ((ROMWindow *)(region->userdata))->emulator.chipset.rom_data[offset - 0x80000];
		}, [](MMURegion *region, size_t offset, uint8_t data) {
			ROMWindow *rom_window = (ROMWindow *)(region->userdata);
			if (rom_window->strict_memory)
			{
				logger::Info("ROM::[region write lambda]: attempt to write %02hhX to %06zX\n", data, offset);
				rom_window->emulator.HandleMemoryError();
			}
		}, emulator);

		strict_memory = emulator.argv_map.find("strict_memory") != emulator.argv_map.end();
	}

	void ROMWindow::Uninitialise()
	{
	}

	void ROMWindow::Tick()
	{
	}

	void ROMWindow::Frame()
	{
	}

	void ROMWindow::UIEvent(SDL_Event &event)
	{
		(void)event;
	}
}
