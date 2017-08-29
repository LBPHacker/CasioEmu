#include "MMURegion.hpp"

namespace casioemu
{
	bool MMURegion::Includes(size_t offset) const
	{
		return offset >= base && offset < base + size;
	}

	bool MMURegion::operator <(const MMURegion &other) const
	{
		if (size == 0 && other.Includes(base))
			return false;
		if (other.size == 0 && Includes(other.base))
			return false;

		return base < other.base;
	}
}

