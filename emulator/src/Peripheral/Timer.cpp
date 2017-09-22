#include "Timer.hpp"

#include "../Logger.hpp"
#include "../Chipset/MMU.hpp"
#include "../Emulator.hpp"
#include "../Chipset/Chipset.hpp"

namespace casioemu
{
	void Timer::Initialise()
	{
		interrupt_source.Setup(9, emulator);

		region_timeout.Setup(0xF020, 2, "Timer/Timeout", &data_timeout, MMURegion::DefaultRead<uint16_t>, MMURegion::DefaultWrite<uint16_t>, emulator);
		region_F022.Setup(0xF022, 2, "Timer/Unknown/F022*2", &data_F022, MMURegion::DefaultRead<uint16_t>, MMURegion::DefaultWrite<uint16_t>, emulator);
		region_F024.Setup(0xF024, 1, "Timer/Unknown/F024*1", &data_F024, MMURegion::DefaultRead<uint8_t>, MMURegion::DefaultWrite<uint8_t>, emulator);

		region_control.Setup(0xF025, 1, "Timer/Control", this, [](MMURegion *region, size_t offset) {
			Timer *self = (Timer *)(region->userdata);
			return self->data_control;
		}, [](MMURegion *region, size_t offset, uint8_t data) {
			Timer *self = (Timer *)(region->userdata);
			self->data_control = data;
			if (data & 0x01)
			{
				self->int_ticks_left = self->data_timeout;
				if (self->int_ticks_left < 8)
				{
					logger::Info("[TIMER] clamping timeout to 8 (was %i)\n", self->int_ticks_left);
					self->int_ticks_left = 8;
				}

				self->counting = true;
			}
		}, emulator);

		counting = false;

		ext_to_int_counter = 0;
		ext_to_int_next = 0;
		ext_to_int_int_done = 0;
		DivideTicks();
	}

	void Timer::Tick()
	{
		if (ext_to_int_counter == ext_to_int_next)
			DivideTicks();
		++ext_to_int_counter;

		if (counting && !int_ticks_left)
			interrupt_source.TryRaise();
	}

	void Timer::TickAfterInterrupts()
	{
		if (counting && !int_ticks_left && interrupt_source.Success())
			counting = false;
	}

	void Timer::DivideTicks()
	{
		++ext_to_int_int_done;
		if (ext_to_int_int_done == ext_to_int_frequency)
		{
			ext_to_int_int_done = 0;
			ext_to_int_counter = 0;
		}
		ext_to_int_next = emulator.GetCyclesPerSecond() * (ext_to_int_int_done + 1) / ext_to_int_frequency;

		if (counting && int_ticks_left)
			--int_ticks_left;
	}
}

