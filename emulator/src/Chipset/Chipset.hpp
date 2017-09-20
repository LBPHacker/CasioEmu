#pragma once
#include "../Config.hpp"

#include <string>
#include <vector>
#include <forward_list>
#include <SDL2/SDL.h>

namespace casioemu
{
	class Emulator;
	class CPU;
	class MMU;
	class Peripheral;

	class Chipset
	{
		enum InterruptIndex
		{
			INT_CHECKFLAG,
			INT_RESET,
			INT_BREAK,
			INT_EMULATOR,
			INT_NONMASKABLE,
			INT_MASKABLE,
			INT_SOFTWARE = 64,
			INT_COUNT = 128
		};

		enum RunMode
		{
			RM_STOP,
			RM_HALT,
			RM_RUN
		};
		RunMode run_mode;

		std::forward_list<Peripheral *> peripherals;

		/**
		 * A bunch of internally used methods for encapsulation purposes.
		 */
		size_t pending_interrupt_count;
		bool interrupts_active[INT_COUNT];
		void AcceptInterrupt();
		void RaiseSoftware(size_t index);
		void ConstructPeripherals();

	public:
		Chipset(Emulator &emulator);
		~Chipset();

		Emulator &emulator;
		CPU &cpu;
		MMU &mmu;
		std::vector<unsigned char> rom_data;

		/**
		 * This exists because the Emulator that owns this Chipset is not ready
		 * to supply a ROM path upon construction. It has to call `LoadROM` later
		 * in its contstructor.
		 */
		void SetupInternals();

		/**
		 * See 1.3.7 in the nX-U8 manual.
		 */
		void Reset();
		void Break();
		void Halt();
		void Stop();
		void RaiseEmulator();
		void RaiseNonmaskable();
		void RaiseMaskable(size_t index);

		void Tick();
		void Frame();
		void UIEvent(SDL_Event &event);

		friend class CPU;
	};
}

