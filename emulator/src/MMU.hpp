#pragma once

#include "Config.hpp"

#include "MMURegion.hpp"

#include <cstdint>
#include <set>
#include <string>

namespace casioemu
{
	class Emulator;

	class MMU
	{
	private:
		Emulator &emulator;

		std::set<MMURegion> regions;
		bool FindRegion(size_t offset, MMURegion &region_out);

	public:
		MMU(Emulator &emulator);
		uint64_t ReadCode(size_t offset, size_t length);
		uint64_t ReadData(size_t offset, size_t length);
		void WriteData(size_t offset, size_t length, uint64_t data);

		void RegisterRegion(MMURegion region);
		void UnregisterRegion(MMURegion region);
	};
}

