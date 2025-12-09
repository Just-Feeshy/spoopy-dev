#include "../spoopy_system_info.h"
#include <memory/spoopy_memory.h>
#include <utils/spoopy_misc_math.h>
#include <spoopy_log.h>
#include <utils/assert.h>

#ifdef _MSC_VER
#include <malloc.h>
#endif

#include <mimalloc.h>

// TODO (Framework): Have support for users to define their own memory allocators

void spoopy_heap_free(void* ptr) {
	mi_free(ptr);
}

void* spoopy_heap_alloc(size_t size) {
	return spoopy_aligned_alloc(SPOOPY_MAX_ALIGN, size);
}

void* spoopy_aligned_alloc(size_t alignment, size_t size) {
	alignment = spoopy_max(alignment, SPOOPY_MAX_ALIGN);
	return mi_calloc_aligned(1, size, alignment);
}

void* spoopy_heap_realloc(void* ptr, size_t size) {
	assert(size > 0);
	return mi_realloc(ptr, size);
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
