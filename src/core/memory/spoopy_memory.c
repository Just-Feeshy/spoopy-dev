#include "spoopy_core.h"
#include <memory/spoopy_memory.h>

// TODO: Have support for users to define their own memory allocators, and use them here.
// This is becoming a bit of a mess with all the platform-specific stuff.

#ifdef _MSC_VER
#include <malloc.h>
#endif

#ifdef SPOOPY_SUPPORT_SDL_MEM
#include <SDL3/SDL_stdinc.h>
#endif


#if defined(__clang__) && __clang_major__ < 11
static void* aligned_alloc(size_t alignment, size_t size) {
    // Clang < 11 does not support aligned_alloc, so we use posix_memalign

    void* ptr = NULL;
    int result = posix_memalign(&ptr, alignment, size);
    return (result == 0) ? ptr : NULL;
}
#endif

void spoopy_heap_free(void* ptr) {
#ifdef _WIN32
    _aligned_free(ptr);
#else
    free(ptr);
#endif
}

void* spoopy_heap_alloc(size_t size) {
#ifdef SPOOPY_SUPPORT_SDL_MEM
    void* ptr = SDL_malloc(size);
#else
    void* ptr = malloc(size);
#endif

    if(SPOOPY_UNLIKELY(!ptr)) {
        SPOOPY_LOG_ERROR("Failed to allocate %zu bytes of memory", size);
        return NULL;
    }

    return ptr;
}

void* spoopy_stack_alloc(size_t size) {
#if SPOOPY_ALLOCA_AVAILABLE == 0
    SPOOPY_LOG_ERROR("Stack allocation is not supported on this platform");
    return NULL;
#endif

#ifdef SPOOPY_HAS_BUILTIN_ALLOCA
    return __builtin_alloca(size);
#elif defined(SPOOPY_HAS_MSVC_ALLOCA)
    return _alloca(size);
#elif defined(SPOOPY_HAS_ALLOCA_H) || defined(SPOOPY_HAS_STDLIB_ALLOCA) || defined(SPOOPY_HAS_MALLOC_H_ALLOCA)
    return alloca(size);
#endif
}
