#pragma once
#include "Config.hpp"

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

		MMURegion **segment_0_dispatch;

	public:
		MMU(Emulator &emulator);
		~MMU();
		uint64_t ReadCode(size_t offset, size_t length);
		uint64_t ReadData(size_t offset, size_t length);
		void WriteData(size_t offset, size_t length, uint64_t data);

		void RegisterRegion(MMURegion *region);
		void UnregisterRegion(MMURegion *region);
	};
}

