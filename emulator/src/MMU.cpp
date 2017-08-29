#include "MMU.hpp"

#include "Emulator.hpp"
#include "Chipset.hpp"
#include "Logger.hpp"

namespace casioemu
{
	MMU::MMU(Emulator &_emulator) : emulator(_emulator)
	{
	}

	uint64_t MMU::ReadCode(size_t offset, size_t length)
	{
		uint64_t result = 0;
		for (size_t ix = 0; ix != length; ++ix)
			result |= uint64_t(emulator.chipset.rom_data[offset + ix]) << (8 * ix);
		return result;
	}

	uint64_t MMU::ReadData(size_t offset, size_t length)
	{
		MMURegion look_for_this = {offset, 0};
		auto find_result = regions.find(look_for_this);
		if (find_result == regions.end())
		{
			logger::Info("MMU::ReadData: unmapped read of size %zu from %06zX\n", length, offset);
			return 0;
		}

		return (*find_result).read((*find_result), offset, length);
	}

	void MMU::WriteData(size_t offset, size_t length, uint64_t data)
	{
		MMURegion look_for_this = {offset, 0};
		auto find_result = regions.find(look_for_this);
		if (find_result == regions.end())
		{
			logger::Info("MMU::WriteData: unmapped write of size %zu to %06zX\n", length, offset);
			return;
		}

		(*find_result).write((*find_result), offset, length, data);
	}

	void MMU::RegisterRegion(MMURegion new_region)
	{
		if (!new_region.size)
			PANIC("attempt to register empty region\n");

		regions.insert(new_region);
	}

	void MMU::UnregisterRegion(MMURegion region)
	{
		if (!region.size)
			PANIC("attempt to unregister empty region\n");

		regions.erase(region);
	}

	bool MMU::FindRegion(size_t offset, MMURegion &region_out)
	{
		MMURegion look_for_this = {offset, 0};
		auto find_result = regions.find(look_for_this);
		if (find_result == regions.end())
			return false;

		region_out = *find_result;
		return true;
	}
}

