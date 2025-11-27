#pragma once

#include <stdio.h>
#include <stdlib.h>

#define FATAL_ERROR(msg, ...) { printf(msg "\n", ##__VA_ARGS__); exit(-1); }
