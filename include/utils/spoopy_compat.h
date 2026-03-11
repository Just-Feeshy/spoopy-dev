#pragma once

#include <stdint.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>
#include <limits.h>
#include <assert.h>

#ifdef __cplusplus
extern "C" {
#endif


/* =============================================================================
 * SDL & CGLM COMPATIBILITY
 * ============================================================================= */

#define SDL_GPU_DISABLE
#define CGLM_CONFIG_CLIP_CONTROL CGLM_CLIP_CONTROL_RH_ZO
#define SPOOPY_HAS_SDL_WINDOW_SUPPORT


/* =============================================================================
 * FEATURE DETECTION
 * ============================================================================= */

#ifdef __has_include
#define SPOOPY_HAS_INCLUDE(header) __has_include(header)
#else
#define SPOOPY_HAS_INCLUDE(header) 0
#endif


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

#if !defined(SPOOPY_ALIGN_AS)

#if SPOOPY_HAVE_ATTRIBUTES
  #define SPOOPY_ALIGN_AS(align) __attribute__((aligned(align)))
#elif defined(_MSC_VER)
  #define SPOOPY_ALIGN_AS(align) __declspec(align(align))
#elif __STDC_VERSION__ >= 201112L
  #define SPOOPY_ALIGN_AS(align) _Alignas(align)
#else
  #define SPOOPY_ALIGN_AS(align)
#endif

#endif // If SPOOPY_ALIGN_AS is not defined already


#if SPOOPY_HAVE_ATTRIBUTES

#define SPOOPY_ATTR(...) __attribute__((__VA_ARGS__))
#define SPOOPY_ATTR_SIZE(...) __attribute__((alloc_size(__VA_ARGS__)))
#define SPOOPY_ATTR_ALLOC_ALIGN(arg_index) __attribute__((alloc_align(arg_index)))
#define SPOOPY_ATTR_WEAK __attribute__((weak))
#define SPOOPY_ATTR_PURE __attribute__((pure))
#define SPOOPY_ATTR_CONST __attribute__((const))
#define SPOOPY_ATTR_NORETURN __attribute__((noreturn))
#define SPOOPY_ATTR_UNUSED __attribute__((unused))
#define SPOOPY_ATTR_USED __attribute__((used))
#define SPOOPY_ATTR_WARN_UNUSED_RESULT __attribute__((warn_unused_result))
#define SPOOPY_FORCE_INLINE static inline __attribute__((always_inline))

#else

#define SPOOPY_ATTR(...)
#define SPOOPY_ATTR_SIZE(...)
#define SPOOPY_ATTR_PURE
#define SPOOPY_ATTR_CONST
#define SPOOPY_ATTR_NORETURN
#define SPOOPY_ATTR_UNUSED
#define SPOOPY_ATTR_USED
#define SPOOPY_ATTR_WARN_UNUSED_RESULT

#define SPOOPY_ATTR_WEAK
#if defined(_MSC_VER)
#define SPOOPY_FORCE_INLINE static __forceinline
#else
#define SPOOPY_FORCE_INLINE static inline
#endif

#endif

#if defined(__cplusplus)
#define SPOOPY_RESTRICT
#elif defined(_MSC_VER)
#define SPOOPY_RESTRICT __restrict
#else
#define SPOOPY_RESTRICT restrict
#endif

#define SPOOPY_UNUSED(x) (void)(x)

#define SPOOPY_WEAK SPOOPY_ATTR_WEAK

#if defined(__GNUC__) || defined(__clang__)
#define SPOOPY_UNREACHABLE() do { \
	assert(0 && "This code should never be reachable"); \
	__builtin_unreachable(); \
} while(0)
#else
#define SPOOPY_UNREACHABLE() do { \
	assert(0 && "This code should never be reachable"); \
} while(0)
#endif


/* =============================================================================
 * COMPILER DIAGNOSTIC HELPERS
 * ============================================================================= */

#if defined(_MSC_VER)

#define SPOOPY_DIAGNOSTIC_PUSH __pragma(warning(push))
#define SPOOPY_DIAGNOSTIC_POP  __pragma(warning(pop))
#define SPOOPY_DIAGNOSTIC_IGNORE(option) __pragma(warning(disable : option))

#elif defined(__clang__)

#define SPOOPY_DIAG_PUSH() _Pragma("clang diagnostic push")
#define SPOOPY_DIAG_POP()  _Pragma("clang diagnostic pop")
#define SPOOPY_DIAG_IGNORE(option) _Pragma("clang diagnostic ignored \"" option "\"")
#define SPOOPY_DIAG_IGNORE_CAST_ALIGN() _Pragma("clang diagnostic ignored \"-Wcast-align\"")
#define SPOOPY_DIAG_IGNORE_CAST_QUAL()  _Pragma("clang diagnostic ignored \"-Wcast-qual\"")
#define SPOOPY_DIAG_

#elif defined(__GNUC__)

#define SPOOPY_DIAG_PUSH() _Pragma("GCC diagnostic push")
#define SPOOPY_DIAG_POP()  _Pragma("GCC diagnostic pop")
#define SPOOPY_DIAG_IGNORE(option) _Pragma("GCC diagnostic ignored \"" option "\"")
#define SPOOPY_DIAG_IGNORE_CAST_ALIGN() _Pragma("GCC diagnostic ignored \"-Wcast-align\"")
#define SPOOPY_DIAG_IGNORE_CAST_QUAL()  _Pragma("GCC diagnostic ignored \"-Wcast-qual\"")

#else

#define SPOOPY_DIAG_PUSH()
#define SPOOPY_DIAG_POP()
#define SPOOPY_DIAG_IGNORE(option)
#define SPOOPY_DIAG_IGNORE_CAST_ALIGN()
#define SPOOPY_DIAG_IGNORE_CAST_QUAL()

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
