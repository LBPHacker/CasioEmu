#include "ROMWindow.hpp"

#include "../Logger.hpp"
#include "../Chipset/MMU.hpp"
#include "../Emulator.hpp"
#include "../Chipset/Chipset.hpp"

namespace casioemu
{
	void ROMWindow::Initialise()
	{
		region_0.Setup(0x00000, 0x08000, "ROM/Segment0", &emulator, [](MMURegion *region, size_t offset) {
			return ((Emulator *)(region->userdata))->chipset.rom_data[offset];
		}, [](MMURegion *region, size_t offset, uint8_t data) {
			logger::Info("ROM::[region write lambda]: attempt to write %02hhX to %06zX\n", data, offset);
		}, emulator);

		region_1.Setup(0x10000, 0x10000, "ROM/Segment1", &emulator, [](MMURegion *region, size_t offset) {
			return ((Emulator *)(region->userdata))->chipset.rom_data[offset];
		}, [](MMURegion *region, size_t offset, uint8_t data) {
			logger::Info("ROM::[region write lambda]: attempt to write %02hhX to %06zX\n", data, offset);
		}, emulator);

		region_8.Setup(0x80000, 0x10000, "ROM/Segment8", &emulator, [](MMURegion *region, size_t offset) {
			return ((Emulator *)(region->userdata))->chipset.rom_data[offset - 0x80000];
		}, [](MMURegion *region, size_t offset, uint8_t data) {
			logger::Info("ROM::[region write lambda]: attempt to write %02hhX to %06zX\n", data, offset);
		}, emulator);
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
	}
}

