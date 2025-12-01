#pragma once

// Platform headers
#include <crtdbg.h>

// Standard headers
#include <stdio.h>
#include <stdlib.h>

#define FATAL_ERROR(msg, ...) { printf(msg "\n", ##__VA_ARGS__); exit(-1); }
#define DBG_BREAK _CrtDbgBreak
