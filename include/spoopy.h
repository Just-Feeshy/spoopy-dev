#ifndef SPOOPY_H
#define SPOOPY_H

#ifndef SPOOPY_SUPPORT_SDL_THREADS
#define SPOOPY_SUPPORT_SDL_THREADS 1
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

// Include all necessary headers to act as an API for the developer
#ifdef SPOOPY_MAIN_PROJECT

#include <memory/spoopy_memory.h>
#include <spoopy_shader.h>
#include <spoopy_thread.h>
#include <spoopy_log.h>
#include <spoopy_video.h>
#include <spoopy_app.h>

#endif // SPOOPY_MAIN_PROJECT

#endif
