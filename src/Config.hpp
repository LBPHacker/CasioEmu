#pragma once

#include <stdio.h>
#include <stdlib.h>

#define Panic(...) (printf("%s:%i: ", __FILE__, __LINE__), printf(__VA_ARGS__), exit(1))

