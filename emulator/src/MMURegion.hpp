#pragma once
#include "Config.hpp"

#include <cstdint>
#include <string>

namespace casioemu
{
	struct MMURegion
	{
		size_t base, size;
		std::string description;
		void *userdata;
		uint8_t (*read)(MMURegion *, size_t);
		void (*write)(MMURegion *, size_t, uint8_t);
	};
}

