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

		region_F048.Setup(0xF048, 8, "Miscellaneous/F048*8", &data_F048, MMURegion::DefaultRead<uint64_t>, MMURegion::DefaultWrite<uint64_t>, emulator);

		region_F220.Setup(0xF220, 4, "Miscellaneous/F220*4", &data_F220, MMURegion::DefaultRead<uint32_t>, MMURegion::DefaultWrite<uint32_t>, emulator);
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

