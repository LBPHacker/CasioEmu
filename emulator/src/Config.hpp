#pragma once

#include <stdio.h>
#include <stdlib.h>

#define PANIC(...) (printf("%s:%i: ", __FILE__, __LINE__), printf(__VA_ARGS__), exit(1))
#define MODEL_DEF_NAME "model.def"

