#ifndef SPOOPY_H
#define SPOOPY_H

#ifndef SPOOPY_SUPPORT_SDL_THREADS
#define SPOOPY_SUPPORT_SDL_THREADS 1
#endif

#ifndef SPOOPY_FUNC_CORE

#ifdef _WIN32

#define SPOOPY_FUNC_CORE __declspec(dllexport)
#define SPOOPY_FUNC_CORE __declspec(dllimport)

#else

#define SPOOPY_FUNC_CORE __attribute__((visibility("default")))

#endif // OS check

#endif // SPOOPY_FUNC_CORE (if not defined)

#include <utils/spoopy_compat.h>
#include <utils/assert.h>

#endif // SPOOPY_H
