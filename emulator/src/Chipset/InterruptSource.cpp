#include "InterruptSource.hpp"

#include "../Emulator.hpp"
#include "Chipset.hpp"

namespace casioemu
{
	void InterruptSource::Setup(size_t _interrupt_index, Emulator &_emulator)
	{
		if (setup_done)
			PANIC("Setup invoked twice\n");

		interrupt_index = _interrupt_index;
		emulator = &_emulator;

		setup_done = true;
	}

	bool InterruptSource::TryRaise()
	{
		if (!setup_done)
			PANIC("Setup not invoked\n");

		raise_success = emulator->chipset.TryRaiseMaskable(interrupt_index);
		return raise_success;
	}

	bool InterruptSource::Success()
	{
		if (!setup_done)
			PANIC("Setup not invoked\n");

		return raise_success && emulator->chipset.GetInterruptPendingSFR(interrupt_index);
	}
}

