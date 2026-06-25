#pragma once

#include <spoopy_log.h>

#if SPOOPY_DEBUG
#define assert_unlikely(expr) \
	do { \
		if (SPOOPY_UNLIKELY(!(expr))) { \
			SPOOPY_LOG_ERROR("Assertion failed: %s, file %s, line %d", #expr, __FILE__, __LINE__); \
			abort(); \
		} \
	} while(0)
#else
#define assert_unlikely(expr) ((void)sizeof(expr))
#endif

#if SPOOPY_DEBUG
#define spoopy_assert(expr) \
	do { \
		if (!(expr)) { \
			SPOOPY_LOG_ERROR("Assertion failed: %s, file %s, line %d", #expr, __FILE__, __LINE__); \
			abort(); \
		} \
	} while(0)
#else
#define spoopy_assert(expr) ((void)sizeof(expr))
#endif

#ifdef assert
#undef assert
#endif
#define assert(expr) spoopy_assert(expr)

static inline void runtime_assert(bool expr, const char* message) {
    if(!expr) {
        SPOOPY_LOG_ERROR("Runtime assertion failed: %s", message);
        abort();
    }
}

#if defined(__STDC_VERSION__)

#if __STDC_VERSION__ <= 201710L
#define static_assert _Static_assert
#endif // __STDC_VERSION__ check

#endif // __STDC_VERSION__ check

/* =============================================================================
 * ASSUME MACROS
 *
 * assume() tells the compiler to assume a condition is true for optimization.
 * - In debug builds: Acts like an assertion (checks and aborts if false)
 * - In release builds: Provides optimization hints to the compiler
 * ============================================================================= */

#if !SPOOPY_DEBUG

#if defined(__clang__)
#define spoopy_assume(expr) __builtin_assume(!!(expr))
#elif defined(__GNUC__)
#define spoopy_assume(expr) \
	do { \
		if (!(expr)) \
			__builtin_unreachable(); \
	} while(0)
#elif defined(_MSC_VER)
#define spoopy_assume(expr) __assume(expr)
#else
#define spoopy_assume(expr) ((void)sizeof(expr))
#endif

#else

#define spoopy_assume(expr) \
	do { \
		if (!(expr)) { \
			SPOOPY_LOG_ERROR("Assumption failed: %s, file %s, line %d", #expr, __FILE__, __LINE__); \
			abort(); \
		} \
	} while(0)

#endif

#define assume(expr) spoopy_assume(expr)
