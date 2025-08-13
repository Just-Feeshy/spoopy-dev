#ifndef SPOOPY_COMPAT_H
#define SPOOPY_COMPAT_H

#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdarg.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif


/* =============================================================================
 * SDL COMPATIBILITY
 * ============================================================================= */

#define SDL_GPU_DISABLE


/* =============================================================================
 * COMPILER DETECTION
 * ============================================================================= */

#if defined(__GNUC__) || defined(__clang__)

#define SPOOPY_HAVE_BUILTIN_EXPECT 1
#define SPOOPY_HAVE_ATTRIBUTES 1

#else

#define SPOOPY_HAVE_BUILTIN_EXPECT 0
#define SPOOPY_HAVE_ATTRIBUTES 0

#endif


/* =============================================================================
 * BRANCH PREDICTION HINTS
 * ============================================================================= */

#if SPOOPY_HAS_BUILTIN_EXPECT

#define SPOOPY_LIKELY(expr)   __builtin_expect(!!(expr), 1)
#define SPOOPY_UNLIKELY(expr) __builtin_expect(!!(expr), 0)

#else

#define SPOOPY_LIKELY(expr)   (expr)
#define SPOOPY_UNLIKELY(expr) (expr)

#endif


/* =============================================================================
 * FUNCTION ATTRIBUTES
 * ============================================================================= */

#if SPOOPY_HAVE_ATTRI

#define SPOOPY_ATTR(...) __attribute__((__VA_ARGS__))
#define SPOOPY_ATTR_SIZE(...) __attribute__((alloc_size(__VA_ARGS__)))
#define SPOOPY_ATTR_ALIGN(index) __attribute__ ((alloc_align(index)))
#define SPOOPY_ATTR_PURE __attribute__((pure))
#define SPOOPY_ATTR_CONST __attribute__((const))
#define SPOOPY_ATTR_NORETURN __attribute__((noreturn))
#define SPOOPY_ATTR_UNUSED __attribute__((unused))
#define SPOOPY_ATTR_USED __attribute__((used))
#define SPOOPY_ATTR_WARN_UNUSED_RESULT __attribute__((warn_unused_result))

#else

#define SPOOPY_ATTR(...)
#define SPOOPY_ATTR_SIZE(...)
#define SPOOPY_ATTR_ALIGN(index)
#define SPOOPY_ATTR_PURE
#define SPOOPY_ATTR_CONST
#define SPOOPY_ATTR_NORETURN
#define SPOOPY_ATTR_UNUSED
#define SPOOPY_ATTR_USED
#define SPOOPY_ATTR_WARN_UNUSED_RESULT

#endif


#if defined(__GNUC__) && !defined(__clang__) && __GNUC__ >= 11

#define SPOOPY_HAVE_ATTR_MALLOC_ARGS 1
#define SPOOPY_ATTR_DEALLOC(deallocator, arg_index) \
    __attribute__((malloc(deallocator, arg_index)))

#else

#define SPOOPY_HAVE_ATTR_MALLOC_ARGS 0
#define SPOOPY_ATTR_DEALLOC(deallocator, arg_index)

#endif


/* =============================================================================
 * MSVC BUILTIN COMPATIBILITY SHIMS
 * ============================================================================= */

#ifdef _MSC_VER

#include <intrin.h>

    static inline int __builtin_ctz(uint32_t x) {
        unsigned long index;
        return _BitScanForward(&index, x) ? (int)index : 32;
    }

    static inline int __builtin_ctzll(uint64_t x) {
        unsigned long index;
#ifdef _WIN64
            return _BitScanForward64(&index, x) ? (int)index : 64;
#else
            /* 32-bit fallback */
            if (_BitScanForward(&index, (uint32_t)x)) {
                return (int)index;
            }
            if (_BitScanForward(&index, (uint32_t)(x >> 32))) {
                return (int)index + 32;
            }
            return 64;
#endif
    }

    static inline int __builtin_ctzl(unsigned long x) {
        return sizeof(x) == 8 ? __builtin_ctzll(x) : __builtin_ctz((uint32_t)x);
    }

    static inline int __builtin_clz(uint32_t x) {
        unsigned long index;
        return _BitScanReverse(&index, x) ? (31 - (int)index) : 32;
    }

    static inline int __builtin_clzll(uint64_t x) {
        unsigned long index;
        #ifdef _WIN64
            return _BitScanReverse64(&index, x) ? (63 - (int)index) : 64;
        #else
            /* 32-bit fallback */
            uint32_t high = (uint32_t)(x >> 32);
            if (_BitScanReverse(&index, high)) {
                return 31 - (int)index;
            }
            uint32_t low = (uint32_t)x;
            if (_BitScanReverse(&index, low)) {
                return 63 - (int)index;
            }
            return 64;
        #endif
    }

    static inline int __builtin_clzl(unsigned long x) {
        return sizeof(x) == 8 ? __builtin_clzll(x) : __builtin_clz((uint32_t)x);
    }

    static inline int __builtin_popcount(uint32_t x) {
        return (int)__popcnt(x);
    }

    static inline int __builtin_popcountll(uint64_t x) {
        #ifdef _WIN64
            return (int)__popcnt64(x);
        #else
            return __builtin_popcount((uint32_t)x) +
                   __builtin_popcount((uint32_t)(x >> 32));
        #endif
    }

    static inline int __builtin_popcountl(unsigned long x) {
        return sizeof(x) == 8
        ? __builtin_popcountll(x)
        : __builtin_popcount((uint32_t)x);
    }
#endif // _MSC_VER

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // SPOOPY_COMPAT_H
