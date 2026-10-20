#pragma once

#include <cstdio>

// GB_LOG: printf-style tracing that exists only in debug builds.
// CMake defines NDEBUG automatically for Release (and not for Debug),
// so no build configuration is needed to switch it on/off.
//
// Usage: GB_LOG("opcode 0x%x\n", opcode);
// In release builds this compiles to nothing.
#ifndef NDEBUG
#define GB_LOG(...) std::printf(__VA_ARGS__)
#else
#define GB_LOG(...) ((void)0)
#endif
