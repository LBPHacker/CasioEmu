#pragma once
#include "../Config.hpp"

#include "Peripheral.hpp"
#include "../Chipset/MMURegion.hpp"
#include "../Chipset/InterruptSource.hpp"

namespace casioemu
{
	class Timer : public Peripheral
	{
		MMURegion region_control, region_timeout, region_F022, region_F024;
		uint16_t data_timeout, data_F022;
		uint8_t data_control, data_F024;
		uint16_t int_ticks_left;

		InterruptSource interrupt_source;

		bool counting, counting_request;
		unsigned int ext_to_int_counter, ext_to_int_next, ext_to_int_int_done;
		const unsigned int ext_to_int_frequency = 10000;

	public:
		using Peripheral::Peripheral;

		void Initialise();
		void Tick();
		void TickAfterInterrupts();
		void DivideTicks();
	};
}

