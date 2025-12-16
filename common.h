#pragma once

// Platform headers
#include <crtdbg.h>

// Standard headers
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define FATAL_ERROR(msg, ...) { printf(msg "\n", ##__VA_ARGS__); exit(-1); }
#define DBG_BREAK _CrtDbgBreak

typedef uint8_t u8;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int64_t i64;
