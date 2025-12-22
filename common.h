#pragma once

// Platform headers
#ifdef _MSC_VER
#include <crtdbg.h>
#endif

// Standard headers
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define FATAL_ERROR(msg, ...) { printf(msg "\n", ##__VA_ARGS__); exit(-1); }

#ifdef _MSC_VER
#define DBG_BREAK() _CrtDbgBreak()
#else
#define DBG_BREAK() __asm__("int3")
#endif

typedef uint8_t u8;
typedef int8_t i8;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int64_t i64;
