#ifndef SPOOPY_MEMORY_H
#define SPOOPY_MEMORY_H

#include <spoopy.h>

#ifdef __cplusplus
extern "C" {
#endif

enum spoopy_memory_type {
    spoopy_heap,
    spoopy_stack,
    spoopy_aligned,
};

SPOOPY_FUNC_CORE inline void* spoopy_stack_alloc(size_t size)
    SPOOPY_ATTR_SIZE(1);

SPOOPY_FUNC_CORE void* spoopy_heap_alloc(size_t size)
    SPOOPY_ATTR(malloc)
    SPOOPY_ATTR_DEALLOC(spoopy_heap_free, 1)
    SPOOPY_ATTR_DEALLOC(SPOOPY_CORE_HEAP_FREE, 1)
    SPOOPY_ATTR_SIZE(1);

SPOOPY_FUNC_CORE void* spoopy_aligned_alloc(size_t size, size_t alignment)
    SPOOPY_ATTR(malloc)
    SPOOPY_ATTR_DEALLOC(spoopy_heap_free, 1)
    SPOOPY_ATTR_DEALLOC(SPOOPY_CORE_HEAP_FREE, 1)
    SPOOPY_ATTR_SIZE(1);
    SPOOPY_ATTR_ALIGN(2);

static inline char* spoopy_heap_strdup(const char* str) {
    size_t len = strlen(str) + 1;
    return memcpy(spoopy_heap_alloc(len), str, len);
}

SPOOPY_FUNC_CORE void spoopy_heap_free(void* ptr);


// I don't like using too many macros, but this is a good way to handle platform-specific memory allocation, and pretty much everything else.

// TODO: Have an allocator that uses `spoopy_memory_type` where it uses the first 3 bits
// to store the value, giving it the ability to write smart allocators and deallocators
// that can handle different types of memory allocation based on the type passed to it.
// Obviously, this allocator should be a struct with multiple callbacks for each type of memory allocation, thus if I ever want to use Zig to make proper custom allocators,
// it will be easier to implement and create reusable code.
// Also, it will be referred to as `spoopy_allocator_t` or something similar.


#if defined(__GNUC__) || defined(__clang__)

/* GCC/Clang: alloca is built-in */
#define SPOOPY_HAS_BUILTIN_ALLOCA 1

#elif defined(_MSC_VER)

/* MSVC: alloca is available in <malloc.h> */
#include <malloc.h>
#define SPOOPY_HAS_MSVC_ALLOCA 1
#define SPOOPY_ALLOCA_AVAILABLE 1

#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L

/* C99: alloca is available in <alloca.h> */
#define SPOOPY_HAS_C99_ALLOCA 1
#define SPOOPY_ALLOCA_AVAILABLE 1

#endif

#if defined(__linux__) || defined(__GLIBC__)

#include <alloca.h>
#define SPOOPY_HAS_ALLOCA_H 1
#define SPOOPY_ALLOCA_AVAILABLE 1

#elif defined(__APPLE__) || defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__)

#define SPOOPY_HAS_STDLIB_ALLOCA 1
#define SPOOPY_ALLOCA_AVAILABLE 1

#elif defined(_WIN32) && !defined(_MSC_VER)

#include <malloc.h>
#define SPOOPY_HAS_MALLOC_H_ALLOCA 1
#define SPOOPY_ALLOCA_AVAILABLE 1

#else

#define SPOOPY_ALLOCA_AVAILABLE 0

#endif


#ifdef _WIN32
#define SPOOPY_CORE_HEAP_FREE(ptr) _aligned_free(ptr)
#else
#define SPOOPY_CORE_HEAP_FREE(ptr) free(ptr)
#endif

#ifndef SPOOPY_FLEX_ALLOC


// I did this because `__builtin_choose_expr` is not supported for non GNU compilers,
// and I want to keep the code portable for everyone
//
// Your welcome LMAO

#define SPOOPY_FLEX_ALLOC(_type, extra_size, alloc_type) ({\
    runtime_assert((enum spoopy_memory_type)alloc_type >= spoopy_heap \
        && (enum spoopy_memory_type)alloc_type <= spoopy_aligned, \
        "Invalid allocation type for SPOOPY_FLEX_ALLOC, must be a value in the `spoopy_memory_type` enum"); \
    (_type*)alloc_type##_alloc(sizeof(_type) + extra_size); \
});

#endif

#ifdef __cplusplus
}
#endif

#endif // SPOOPY_MEMORY_H (end of file since there are a lot of macros)
