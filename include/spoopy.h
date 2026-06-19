#pragma once

#ifndef SPOOPY_FOURCC_OGGS
#define SPOOPY_FOURCC_OGGS 0x4F676753
#endif

#ifndef SPOOPY_EPSILON
#define SPOOPY_EPSILON 0.00001f
#endif

#ifndef SPOOPY_SUPPORT_SDL_THREADS
#define SPOOPY_SUPPORT_SDL_THREADS 1
#endif

#ifndef SPOOPY_PRIMARY_SCREEN_INDEX
#define SPOOPY_PRIMARY_SCREEN_INDEX 0u
#endif

#ifndef SPOOPY_ARENA_MIN_SIZE
#define SPOOPY_ARENA_MIN_SIZE 4096
#endif

#ifndef SPOOPY_FRAMES_TO_AVERAGE
#define SPOOPY_FRAME_TO_AVERAGE 120
#endif

#ifdef __cplusplus
#define SPOOPY_FLEX_ARR 1
#else
#define SPOOPY_FLEX_ARR
#endif

#ifndef SPOOPY_FUNC_CORE

#ifdef _WIN32

#define SPOOPY_FUNC_CORE __declspec(dllimport)

#elif !defined(_MSC_VER)

#define SPOOPY_FUNC_CORE __attribute__((visibility("default")))

#endif // OS check

#endif // SPOOPY_FUNC_CORE (if not defined)

#include <utils/spoopy_compat.h>
#include <utils/assert.h>
