#include "MMU.hpp"

#include "../Emulator.hpp"
#include "Chipset.hpp"
#include "../Logger.hpp"

namespace casioemu
{
	MMU::MMU(Emulator &_emulator) : emulator(_emulator)
	{
		segment_dispatch = new MMURegion **[0x100];
		for (size_t ix = 0; ix != 0x100; ++ix)
			segment_dispatch[ix] = nullptr;
	}

	MMU::~MMU()
	{
		for (size_t ix = 0; ix != 0x100; ++ix)
			if (segment_dispatch[ix])
				delete[] segment_dispatch[ix];

		delete[] segment_dispatch;
	}

	void MMU::GenerateSegmentDispatch(size_t segment_index)
	{
		segment_dispatch[segment_index] = new MMURegion *[0x10000];
		for (size_t ix = 0; ix != 0x10000; ++ix)
			segment_dispatch[segment_index][ix] = nullptr;
	}

	void MMU::SetupInternals()
	{
		*(MMU **)lua_newuserdata(emulator.lua_state, sizeof(MMU *)) = this;
		lua_newtable(emulator.lua_state);
		lua_pushcfunction(emulator.lua_state, [](lua_State *lua_state) {
			MMU *mmu = *(MMU **)lua_topointer(lua_state, 1);
			lua_pushinteger(lua_state, mmu->ReadCode(lua_tointeger(lua_state, 2)));
			return 1;
		});
		lua_setfield(emulator.lua_state, -2, "__index");
		lua_pushcfunction(emulator.lua_state, [](lua_State *lua_state) {
			return 0;
		});
		lua_setfield(emulator.lua_state, -2, "__newindex");
		lua_setmetatable(emulator.lua_state, -2);
		lua_setglobal(emulator.lua_state, "code");

		*(MMU **)lua_newuserdata(emulator.lua_state, sizeof(MMU *)) = this;
		lua_newtable(emulator.lua_state);
		lua_pushcfunction(emulator.lua_state, [](lua_State *lua_state) {
			MMU *mmu = *(MMU **)lua_topointer(lua_state, 1);
			lua_pushinteger(lua_state, mmu->ReadData(lua_tointeger(lua_state, 2)));
			return 1;
		});
		lua_setfield(emulator.lua_state, -2, "__index");
		lua_pushcfunction(emulator.lua_state, [](lua_State *lua_state) {
			MMU *mmu = *(MMU **)lua_topointer(lua_state, 1);
			mmu->WriteData(lua_tointeger(lua_state, 2), lua_tointeger(lua_state, 3));
			return 0;
		});
		lua_setfield(emulator.lua_state, -2, "__newindex");
		lua_setmetatable(emulator.lua_state, -2);
		lua_setglobal(emulator.lua_state, "data");
	}

	uint16_t MMU::ReadCode(size_t offset)
	{
		if (offset >= (1 << 20))
			PANIC("offset doesn't fit 20 bits\n");
		if (offset & 1)
			PANIC("offset has LSB set\n");

		size_t segment_index = offset >> 16;
		size_t segment_offset = offset & 0xFFFF;

		if (!segment_index)
			return (((uint16_t)emulator.chipset.rom_data[segment_offset + 1]) << 8) | emulator.chipset.rom_data[segment_offset];

		MMURegion **segment = segment_dispatch[segment_index];
		if (!segment)
		{
			logger::Info("code read from offset %04zX of unmapped segment %02zX\n", segment_offset, segment_index);
			emulator.HandleMemoryError();
			return 0;
		}

		MMURegion *region = segment[segment_offset];
		if (!region)
		{
			logger::Info("code read from unmapped offset %04zX of segment %02zX\n", segment_offset, segment_index);
			emulator.HandleMemoryError();
			return 0;
		}

		return (((uint16_t)region->read(region, offset + 1)) << 8) | region->read(region, offset);
	}

	uint8_t MMU::ReadData(size_t offset)
	{
		if (offset >= (1 << 24))
			PANIC("offset doesn't fit 24 bits\n");

		size_t segment_index = offset >> 16;
		size_t segment_offset = offset & 0xFFFF;

		MMURegion **segment = segment_dispatch[segment_index];
		if (!segment)
		{
			logger::Info("read from offset %04zX of unmapped segment %02zX\n", segment_offset, segment_index);
			emulator.HandleMemoryError();
			return 0;
		}

		MMURegion *region = segment[segment_offset];
		if (!region)
		{
			logger::Info("read from unmapped offset %04zX of segment %02zX\n", segment_offset, segment_index);
			emulator.HandleMemoryError();
			return 0;
		}

		return region->read(region, offset);
	}

	void MMU::WriteData(size_t offset, uint8_t data)
	{
		if (offset >= (1 << 24))
			PANIC("offset doesn't fit 24 bits\n");

		size_t segment_index = offset >> 16;
		size_t segment_offset = offset & 0xFFFF;

		MMURegion **segment = segment_dispatch[segment_index];
		if (!segment)
		{
			logger::Info("write to offset %04zX of unmapped segment %02zX (%02zX)\n", segment_offset, segment_index, data);
			emulator.HandleMemoryError();
			return;
		}

		MMURegion *region = segment[segment_offset];
		if (!region)
		{
			logger::Info("write to unmapped offset %04zX of segment %02zX (%02zX)\n", segment_offset, segment_index, data);
			emulator.HandleMemoryError();
			return;
		}

		region->write(region, offset, data);
	}

	void MMU::RegisterRegion(MMURegion *region)
	{
		for (size_t ix = region->base; ix != region->base + region->size; ++ix)
		{
			if (segment_dispatch[ix >> 16][ix & 0xFFFF])
				PANIC("MMU region overlap at %06zX\n", ix);
			segment_dispatch[ix >> 16][ix & 0xFFFF] = region;
		}
	}

	void MMU::UnregisterRegion(MMURegion *region)
	{
		for (size_t ix = region->base; ix != region->base + region->size; ++ix)
		{
			if (!segment_dispatch[ix >> 16][ix & 0xFFFF])
				PANIC("MMU region double-hole at %06zX\n", ix);
			segment_dispatch[ix >> 16][ix & 0xFFFF] = nullptr;
		}
	}
}

