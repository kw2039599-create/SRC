#if !defined(GRANNY_COMPILER_H)
#include "granny_compiler.h"
#endif

// Structure packing explicitly set to 4
#if COMPILER_MSVC || defined(__x86_64__) || defined(__aarch64__) || defined(__ANDROID__)

#pragma pack(push)
#pragma pack(4)

#endif