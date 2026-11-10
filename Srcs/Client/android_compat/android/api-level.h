#pragma once
#ifdef __APPLE__
#include <xlocale.h>
#define __ANDROID_API__ 28
#else
#include_next <android/api-level.h>
#endif
