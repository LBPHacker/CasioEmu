#include "MMU.hpp"

#include "Emulator.hpp"
#include "Chipset.hpp"
#include "Logger.hpp"

namespace casioemu
{
	MMU::MMU(Emulator &_emulator) : emulator(_emulator)
	{
		segment_0_dispatch = new MMURegion *[0x10000];
		for (size_t ix = 0; ix != 0x10000; ++ix)
			segment_0_dispatch[ix] = nullptr;
	}

	MMU::~MMU()
	{
		delete[] segment_0_dispatch;
	}

	uint16_t MMU::ReadCode(size_t offset)
	{
		if (emulator.chipset.rom_data.size() <= offset)
		{
			logger::Info("out-of-bound ROM read from %06zX\n", offset);
			return 0;
		}
		return emulator.chipset.rom_data[offset] | (((uint16_t)emulator.chipset.rom_data[offset + 1]) << 8);
	}

	uint8_t MMU::ReadData(size_t offset)
	{
		if (offset >= 0x10000)
		{
			logger::Info("high segment read from %06zX\n", offset);
			return 0;
		}
		MMURegion *region = segment_0_dispatch[offset];
		if (!region)
		{
			logger::Info("unmapped read from %06zX\n", offset);
			return 0;
		}
		return region->read(region, offset);
	}

	void MMU::WriteData(size_t offset, uint8_t data)
	{
		if (offset >= 0x10000)
		{
			logger::Info("high segment write to %06zX\n", offset);
			return;
		}
		MMURegion *region = segment_0_dispatch[offset];
		if (!region)
		{
			logger::Info("unmapped write to %06zX\n", offset);
			return;
		}
		region->write(region, offset, data);
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

