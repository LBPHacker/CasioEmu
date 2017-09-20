#pragma once
#include "../Config.hpp"

#include <cstdint>
#include <string>

namespace casioemu
{
	class Emulator;

	struct MMURegion
	{
		typedef uint8_t (*ReadFunction)(MMURegion *, size_t);
		typedef void (*WriteFunction)(MMURegion *, size_t, uint8_t);

		size_t base, size;
		std::string description;
		void *userdata;
		ReadFunction read;
		WriteFunction write;
		bool setup_done;
		Emulator *emulator;

		MMURegion();
		~MMURegion();
		void Setup(size_t base, size_t size, std::string description, void *userdata, ReadFunction read, WriteFunction write, Emulator &emulator);

		template<uint8_t R>
		static uint8_t IgnoreRead(MMURegion *region, size_t offset)
		{
			return R;
		}

		static void IgnoreWrite(MMURegion *region, size_t offset, uint8_t data)
		{
		}

		template<typename T, T mask = (T)-1>
		static uint8_t DefaultRead(MMURegion *region, size_t offset)
		{
			T *value = (T *)(region->userdata);
			return ((*value) & mask) >> (offset - region->base);
		}

		template<typename T, T mask = (T)-1>
		static void DefaultWrite(MMURegion *region, size_t offset, uint8_t data)
		{
			T *value = (T *)(region->userdata);
			*value &= ~((T)0xFF) << (offset - region->base);
			*value |= ((T)data) << (offset - region->base);
			*value &= ~mask;
		}
	};
}

