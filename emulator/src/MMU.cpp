#include "MMU.hpp"

#include "Emulator.hpp"
#include "Chipset.hpp"
#include "Logger.hpp"

#define SEGMENT_0_SIZE (1 << 16)

namespace casioemu
{
	MMU::MMU(Emulator &_emulator) : emulator(_emulator)
	{
		segment_0_dispatch = new MMURegion *[SEGMENT_0_SIZE];
		for (size_t ix = 0; ix != SEGMENT_0_SIZE; ++ix)
			segment_0_dispatch[ix] = nullptr;
	}

	MMU::~MMU()
	{
		delete[] segment_0_dispatch;
	}

	uint64_t MMU::ReadCode(size_t offset, size_t length)
	{
		if (emulator.chipset.rom_data.size() < offset + length)
		{
			logger::Info("out-of-bound ROM read of size %zu from %06zX\n", length, offset);
			return 0;
		}
		uint64_t result = 0;
		for (size_t ix = 0; ix != length; ++ix)
			result |= uint64_t(emulator.chipset.rom_data[offset + ix]) << (8 * ix);
		return result;
	}

	uint64_t MMU::ReadData(size_t offset, size_t length)
	{
		if (offset >= SEGMENT_0_SIZE)
		{
			logger::Info("high segment read of size %zu from %06zX\n", length, offset);
			return 0;
		}
		MMURegion *region = segment_0_dispatch[offset];
		if (!region)
		{
			logger::Info("unmapped read of size %zu from %06zX\n", length, offset);
			return 0;
		}
		return region->read(region, offset, length);
	}

	void MMU::WriteData(size_t offset, size_t length, uint64_t data)
	{
		if (offset >= SEGMENT_0_SIZE)
		{
			logger::Info("high segment write of size %zu from %06zX\n", length, offset);
			return;
		}
		MMURegion *region = segment_0_dispatch[offset];
		if (!region)
		{
			logger::Info("unmapped write of size %zu from %06zX\n", length, offset);
			return;
		}
		region->write(region, offset, length, data);
	}

	void MMU::RegisterRegion(MMURegion *region)
	{
		if (!region->size)
			PANIC("attempt to register empty region\n");

		for (size_t ix = region->base; ix != region->base + region->size; ++ix)
		{
			MMURegion *&region_entry = segment_0_dispatch[ix];
			if (region_entry)
				PANIC("region [%06zX, %06zX, '%s'] overlaps with region [%06zX, %06zX, '%s'] at %06zX\n", region_entry->base, region_entry->size, region_entry->description.c_str(), region->base, region->size, region->description.c_str(), ix);
			region_entry = region;
		}
	}

	void MMU::UnregisterRegion(MMURegion *region)
	{
		if (!region->size)
			PANIC("attempt to unregister empty region\n");

		for (size_t ix = region->base; ix != region->base + region->size; ++ix)
		{
			MMURegion *&region_entry = segment_0_dispatch[ix];
			if (!region_entry)
				PANIC("region [%06zX, %06zX, '%s'] is not present at %06zX\n", region->base, region->size, region->description.c_str(), ix);
			region_entry = nullptr;
		}
	}
}

