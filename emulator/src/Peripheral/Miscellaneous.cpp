#include "Miscellaneous.hpp"

#include "../Logger.hpp"
#include "../Emulator.hpp"
#include "../Chipset/Chipset.hpp"
#include "../Chipset/CPU.hpp"

namespace casioemu
{
	void Miscellaneous::Initialise()
	{
		region_dsr.Setup(0xF000, 1, "Miscellaneous/DSR", this, [](MMURegion *region, size_t offset) {
			return (uint8_t)((Miscellaneous *)region->userdata)->emulator.chipset.cpu.reg_dsr;
		}, [](MMURegion *region, size_t offset, uint8_t data) {
			((Miscellaneous *)region->userdata)->emulator.chipset.cpu.reg_dsr = data;
		}, emulator);
	}

	void Miscellaneous::Uninitialise()
	{
	}

	void Miscellaneous::Tick()
	{
	}

	void Miscellaneous::Frame()
	{
	}

	void Miscellaneous::UIEvent(SDL_Event &event)
	{
	}
}

