#include "Logger.hpp"

#include <stdio.h>
#include <stdarg.h>

namespace casioemu
{
	namespace logger
	{
		void Info(const char *format, ...)
		{
			va_list args;
			va_start(args, format);
			vprintf(format, args);
			va_end(args);
		}

		void InfoHere(const char *format, ...)
		{
			printf("%s:%i: in %s: ", __FILE__, __LINE__, FUNCTION_NAME);
			va_list args;
			va_start(args, format);
			vprintf(format, args);
			va_end(args);
		}
	}
}

