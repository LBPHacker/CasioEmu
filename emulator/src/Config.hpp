#pragma once

#include <stdio.h>
#include <stdlib.h>

#ifdef __GNUG__
# define FUNCTION_NAME __PRETTY_FUNCTION__
#else
# define FUNCTION_NAME __func__
#endif
#define PANIC(...) (fprintf(stderr, "%s:%i: in %s: ", __FILE__, __LINE__, FUNCTION_NAME), fprintf(stderr, __VA_ARGS__), exit(1))

#define MODEL_DEF_NAME "model.def"

