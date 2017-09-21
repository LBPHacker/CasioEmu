#pragma once
#include "../Config.hpp"

namespace casioemu
{
	class Emulator;

	class InterruptSource
	{
		Emulator *emulator;
		bool raise_success, setup_done;
		size_t interrupt_index;

	public:
		void Setup(size_t interrupt_index, Emulator &_emulator);
		bool TryRaise();
		bool Success();
	};
}

