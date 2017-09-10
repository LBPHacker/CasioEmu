#pragma once
#include "Config.hpp"

#include <string>

namespace casioemu
{
	namespace logger
	{
		void Info(const char *format, ...);
		void InfoHere(const char *format, ...);
	}
}

