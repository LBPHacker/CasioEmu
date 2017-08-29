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
		uint64_t (*read)(const MMURegion &, size_t, size_t);
		void (*write)(const MMURegion &, size_t, size_t, uint64_t);
		bool Includes(size_t offset) const;
		bool operator <(const MMURegion &other) const;
	};
}

