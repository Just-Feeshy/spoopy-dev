#pragma once

#include <spoopy.h>
#include <utils/spoopy_misc_math.h>
#include <utils/assert.h>

#define SPOOPY_CONTAINER_OF(ptr, type, member) \
    ((type*)((uint8_t*)(ptr) - offsetof(type, member)))

#ifdef __cplusplus
extern "C" {
#endif


#if defined(__MINGW32__) && !defined(__MINGW64__)
#define SPOOPY_MAX_ALIGN 16
static_assert(SPOOPY_MAX_ALIGN % __alignof(max_align_t) == 0, "");
#else
#define SPOOPY_MAX_ALIGN __alignof(max_align_t)
#endif

typedef struct spoopy_header {
	uint16_t is_unique : 1;
	uint16_t unsigned_size : 15;
} spoopy_header_t;

typedef struct spoopy_static_block {
	spoopy_header_t header;
	SPOOPY_ALIGN_AS(SPOOPY_MAX_ALIGN) uint8_t payload[SPOOPY_FLEX_ARR];
} spoopy_static_block_t;

enum spoopy_memory_type {
    spoopy_heap,
    spoopy_stack,
    spoopy_aligned,
	spoopy_static,
};

SPOOPY_FUNC_CORE void spoopy_memory_init_hooks(void);

SPOOPY_FUNC_CORE void* spoopy_heap_alloc(size_t size)
    SPOOPY_ATTR_DEALLOC(spoopy_heap_free, 1)
    SPOOPY_ATTR_SIZE(1);

SPOOPY_FUNC_CORE void* spoopy_aligned_alloc(size_t alignment, size_t size, void* user_data)
    SPOOPY_ATTR_DEALLOC(spoopy_heap_free, 1)
    SPOOPY_ATTR_SIZE(1);

SPOOPY_FUNC_CORE void* spoopy_heap_realloc(void* ptr, size_t size)
	SPOOPY_ATTR_DEALLOC(spoopy_heap_free, 1)
	SPOOPY_ATTR_SIZE(2);


// (CAUTION): spoopy_static_alloc must be freed with spoopy_static_free, not spoopy_heap_free
// If you free a static allocation with spoopy_heap_free, it will cause memory corruption

SPOOPY_FUNC_CORE spoopy_static_block_t* spoopy_static_realloc(void* ptr, size_t size)
	SPOOPY_ATTR_DEALLOC(spoopy_heap_free, 1)
	SPOOPY_ATTR_SIZE(2);

SPOOPY_FUNC_CORE void spoopy_heap_free(void* ptr);
SPOOPY_FUNC_CORE void spoopy_static_free(void* ptr);


SPOOPY_DIAG_PUSH()
SPOOPY_DIAG_IGNORE_CAST_ALIGN();

SPOOPY_FUNC_CORE spoopy_static_block_t* spoopy_static_alloc(size_t size);

static inline spoopy_static_block_t* spoopy_static_block_from_payload(void* ptr) {
	return SPOOPY_CONTAINER_OF(ptr, spoopy_static_block_t, payload);
}

SPOOPY_DIAG_POP()


static inline char* spoopy_heap_strdup(const char* str) {
    const size_t len = strlen(str) + 1;
    return (char*)memcpy(spoopy_heap_alloc(len), str, len);
}

static inline char* spoopy_heap_strndup(const char* str, size_t len) {
    if(!str) {
        return NULL;
    }

    char* out = (char*)spoopy_heap_alloc(len + 1);
    if(!out) {
        return NULL;
    }

    memcpy(out, str, len);
    out[len] = '\0';
    return out;
}

static inline size_t spoopy_align_bound(size_t size, size_t min_alignment) {
	const size_t clamped = spoopy_max(min_alignment, SPOOPY_MAX_ALIGN);
	const size_t aligned_size = (size + (clamped - 1)) & ~(clamped - 1);

	return aligned_size;
}


// Slow, but portable

static inline bool add_overflow_size_t(size_t a, size_t b, size_t* out) {
	if (a > SIZE_MAX - b) {
		return true;
	}

	*out = a + b;
	return false;
}

static inline bool mul_overflow_size_t(size_t a, size_t b, size_t* out) {
	if (a == 0 || b == 0) {
		*out = 0;
		return false;
	}
	if (a > SIZE_MAX / b) {
		return true;
	}

	*out = a * b;
	return false;
}

static inline size_t spoopy_calc_array_size(size_t nmemb, size_t size) {
	size_t array_size;

	if(mul_overflow_size_t(nmemb, size, &array_size)) {
		assert(false && "Array size overflow in spoopy_calc_array_size");
		abort();
	}

	return array_size;
}


// I don't like using too many macros, but this is a good way to handle platform-specific memory allocation, and pretty much everything else.

// I want to have an allocator that uses `spoopy_memory_type` where it uses the first
// 3 bits to store the value, giving it the ability to write smart allocators and
// deallocators that can handle different types of memory allocation based on the
// type passed to it.
// Obviously, this allocator should be a struct with multiple callbacks for each type of memory allocation, thus if I ever want to use Zig to make proper custom allocators,
// it will be easier to implement and create reusable code.
// Also, it will be referred to as `spoopy_allocator_t` or something similar.

#if defined(__GNUC__) || defined(__clang__)

/* GCC/Clang: alloca is built-in */
#define SPOOPY_HAS_BUILTIN_ALLOCA 1
#ifndef SPOOPY_ALLOCA_AVAILABLE
#define SPOOPY_ALLOCA_AVAILABLE 1
#endif

#elif defined(_MSC_VER)

/* MSVC: alloca is available in <malloc.h> */
#include <malloc.h>
#define SPOOPY_HAS_MSVC_ALLOCA 1
#ifndef SPOOPY_ALLOCA_AVAILABLE
#define SPOOPY_ALLOCA_AVAILABLE 1
#endif

#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L

/* C99: alloca is available in <alloca.h> */
#define SPOOPY_HAS_C99_ALLOCA 1
#ifndef SPOOPY_ALLOCA_AVAILABLE
#define SPOOPY_ALLOCA_AVAILABLE 1
#endif

#endif

#if defined(__linux__) || defined(__GLIBC__)

#include <alloca.h>
#define SPOOPY_HAS_ALLOCA_H 1
#ifndef SPOOPY_ALLOCA_AVAILABLE
#define SPOOPY_ALLOCA_AVAILABLE 1
#endif

#elif defined(__APPLE__) || defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__)

#define SPOOPY_HAS_STDLIB_ALLOCA 1
#ifndef SPOOPY_ALLOCA_AVAILABLE
#define SPOOPY_ALLOCA_AVAILABLE 1
#endif

#elif defined(_WIN32) && !defined(_MSC_VER)

#include <malloc.h>
#define SPOOPY_HAS_MALLOC_H_ALLOCA 1
#ifndef SPOOPY_ALLOCA_AVAILABLE
#define SPOOPY_ALLOCA_AVAILABLE 1
#endif

#else

#ifndef SPOOPY_ALLOCA_AVAILABLE
#define SPOOPY_ALLOCA_AVAILABLE 0
#endif

#endif

#if SPOOPY_ALLOCA_AVAILABLE == 0
static inline void* spoopy_stack_alloc(size_t size) {
    SPOOPY_LOG_ERROR("Stack allocation is not supported on this platform");
    (void)size;
    return NULL;
}
#else
    #ifdef SPOOPY_HAS_BUILTIN_ALLOCA
        #define spoopy_stack_alloc(size) __builtin_alloca(size)
    #elif defined(SPOOPY_HAS_MSVC_ALLOCA)
        #define spoopy_stack_alloc(size) _alloca(size)
    #elif defined(SPOOPY_HAS_ALLOCA_H) || defined(SPOOPY_HAS_STDLIB_ALLOCA) || defined(SPOOPY_HAS_MALLOC_H_ALLOCA)
        #define spoopy_stack_alloc(size) alloca(size)
    #endif
#endif


#ifndef SPOOPY_FLEX_ALLOC


// I did this because `__builtin_choose_expr` is not supported for non GNU compilers,
// and I want to keep the code portable for everyone
//
// Your welcome

#define SPOOPY_FLEX_ALLOC(_type, extra_size, alloc_type) ({\
    runtime_assert((enum spoopy_memory_type)alloc_type >= spoopy_heap \
        && (enum spoopy_memory_type)alloc_type <= spoopy_aligned, \
        "Invalid allocation type for SPOOPY_FLEX_ALLOC, must be a value in the `spoopy_memory_type` enum"); \
    alloc_type##_alloc(sizeof(_type) + extra_size); \
});

#endif

#ifdef __cplusplus
}
#endif
