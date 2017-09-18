#pragma once
#include "../Config.hpp"

#include "MMURegion.hpp"

#include <cstdint>
#include <string>

namespace casioemu
{
	class Emulator;

	class MMU
	{
	private:
		Emulator &emulator;

		MMURegion ***segment_dispatch;

	public:
		MMU(Emulator &emulator);
		~MMU();
		void SetupInternals();
		void GenerateSegmentDispatch(size_t segment_index);
		uint16_t ReadCode(size_t offset);
		uint8_t ReadData(size_t offset);
		void WriteData(size_t offset, uint8_t data);

		void RegisterRegion(MMURegion *region);
		void UnregisterRegion(MMURegion *region);
	};
}

