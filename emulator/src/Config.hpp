#pragma once

#include <stdio.h>
#include <stdlib.h>

#ifdef __GNUG__
# define FUNCTION_NAME __PRETTY_FUNCTION__
#else
# define FUNCTION_NAME __func__
#endif
#define PANIC(...) (printf("%s:%i: in %s: ", __FILE__, __LINE__, FUNCTION_NAME), printf(__VA_ARGS__), exit(1))

#define MODEL_DEF_NAME "model.def"

#define PL_ROMWINDOW_BASE 0x0000
#define PL_ROMWINDOW_SIZE 0x8000

#define PL_BATTERYBACKEDRAM_BASE 0x8000
#define PL_BATTERYBACKEDRAM_SIZE 0x0E00

