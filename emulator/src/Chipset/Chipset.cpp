#include "Chipset.hpp"

#include "../Emulator.hpp"
#include "../Logger.hpp"
#include "CPU.hpp"
#include "MMU.hpp"

#include "../Peripheral/ROMWindow.hpp"
#include "../Peripheral/BatteryBackedRAM.hpp"
#include "../Peripheral/Screen.hpp"
#include "../Peripheral/Keyboard.hpp"
#include "../Peripheral/StandbyControl.hpp"
#include "../Peripheral/Miscellaneous.hpp"

#include <fstream>

namespace casioemu
{
	Chipset::Chipset(Emulator &_emulator) : emulator(_emulator), cpu(*new CPU(emulator)), mmu(*new MMU(emulator))
	{
		cpu.SetMemoryModel(CPU::MM_LARGE);

		ConstructPeripherals();
	}

	void Chipset::ConstructPeripherals()
	{
		mmu.GenerateSegmentDispatch(0);
		mmu.GenerateSegmentDispatch(1);
		mmu.GenerateSegmentDispatch(8);

		// * TODO: Add more peripherals here.
		peripherals.push_front(new ROMWindow(emulator));
		peripherals.push_front(new BatteryBackedRAM(emulator));
		peripherals.push_front(new Screen(emulator));
		peripherals.push_front(new Keyboard(emulator));
		peripherals.push_front(new StandbyControl(emulator));
		peripherals.push_front(new Miscellaneous(emulator));
	}

	Chipset::~Chipset()
	{
		for (auto &peripheral : peripherals)
		{
			peripheral->Uninitialise();
			delete peripheral;
		}

		delete &mmu;
		delete &cpu;
	}

	void Chipset::SetupInternals()
	{
		std::ifstream rom_handle(emulator.GetModelFilePath(emulator.GetModelInfo("rom_path")), std::ifstream::binary);
		if (rom_handle.fail())
			PANIC("std::ifstream failed: %s\n", strerror(errno));
		rom_data = std::vector<unsigned char>((std::istreambuf_iterator<char>(rom_handle)), std::istreambuf_iterator<char>());

		for (auto &peripheral : peripherals)
			peripheral->Initialise();

		cpu.SetupInternals();
		mmu.SetupInternals();
	}

	void Chipset::Reset()
	{
		cpu.Reset();

		interrupts_active[INT_RESET] = true;
		pending_interrupt_count = 1;

		run_mode = RM_RUN;
	}

	void Chipset::Break()
	{
		if (cpu.GetExceptionLevel() > 1)
		{
			Reset();
			return;
		}

		if (interrupts_active[INT_BREAK])
			return;
		interrupts_active[INT_BREAK] = true;
		pending_interrupt_count++;
	}

	void Chipset::Halt()
	{
		run_mode = RM_HALT;
	}

	void Chipset::Stop()
	{
		run_mode = RM_STOP;
	}

	void Chipset::RaiseEmulator()
	{
		if (interrupts_active[INT_EMULATOR])
			return;
		interrupts_active[INT_EMULATOR] = true;
		pending_interrupt_count++;
	}

	void Chipset::RaiseNonmaskable()
	{
		if (interrupts_active[INT_NONMASKABLE])
			return;
		interrupts_active[INT_NONMASKABLE] = true;
		pending_interrupt_count++;
	}

	void Chipset::RaiseMaskable(size_t index)
	{
		if (index < INT_MASKABLE || index >= INT_SOFTWARE)
			PANIC("%zu is not a valid maskable interrupt index\n", index);
		if (interrupts_active[index])
			return;
		interrupts_active[index] = true;
		pending_interrupt_count++;
	}

	void Chipset::RaiseSoftware(size_t index)
	{
		index += 0x40;
		if (interrupts_active[index])
			return;
		interrupts_active[index] = true;
		pending_interrupt_count++;
	}

	void Chipset::AcceptInterrupt()
	{
		size_t old_exception_level = cpu.GetExceptionLevel();

		size_t index = 0;
		// * Reset has priority over everything.
		if (interrupts_active[INT_RESET])
			index = INT_RESET;
		// * Software interrupts are immediately accepted.
		if (!index)
			for (size_t ix = INT_SOFTWARE; ix != INT_COUNT; ++ix)
				if (interrupts_active[ix])
				{
					if (old_exception_level > 1) // * TODO: figure out how to handle this
						PANIC("software interrupt while exception level was greater than 1\n");
					index = ix;
					break;
				}
		// * No need to check the old exception level as NMICI has an exception level of 3.
		if (!index && interrupts_active[INT_EMULATOR])
			index = INT_EMULATOR;
		// * No need to check the old exception level as BRK initiates a reset if
		//   the currect exception level is greater than 1.
		if (!index && interrupts_active[INT_BREAK])
			index = INT_BREAK;
		if (!index && interrupts_active[INT_NONMASKABLE] && old_exception_level <= 2)
			index = INT_NONMASKABLE;
		if (!index && old_exception_level <= 1)
			for (size_t ix = INT_MASKABLE; ix != INT_SOFTWARE; ++ix)
				if (interrupts_active[ix])
				{
					index = ix;
					break;
				}

		size_t exception_level;
		switch (index)
		{
		case INT_RESET:
			exception_level = 0;
			break;

		case INT_BREAK:
		case INT_NONMASKABLE:
			exception_level = 2;
			break;

		case INT_EMULATOR:
			exception_level = 3;
			break;

		default:
			exception_level = 1;
			break;
		}

		if (!(index >= INT_MASKABLE && index < INT_SOFTWARE && !cpu.GetMasterInterruptEnable()))
			cpu.Raise(exception_level, index);

		run_mode = RM_RUN;

		// * TODO: introduce delay

		interrupts_active[index] = false;
		pending_interrupt_count--;
	}

	void Chipset::Frame()
	{
		for (auto peripheral : peripherals)
			peripheral->Frame();
	}

	void Chipset::Tick()
	{
		// * TODO: decrement delay counter, return if it's not 0

		if (run_mode != RM_STOP)
			for (auto peripheral : peripherals)
				peripheral->Tick();

		if (pending_interrupt_count)
			AcceptInterrupt();

		if (run_mode == RM_RUN)
			cpu.Next();
	}

	void Chipset::UIEvent(SDL_Event &event)
	{
		for (auto peripheral : peripherals)
			peripheral->UIEvent(event);
	}
}

