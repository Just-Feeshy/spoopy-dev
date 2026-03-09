#pragma once

#include <spoopy_log.h>

#ifdef NDEBUG
#define assert_unlikely(expr) ((void)sizeof(expr))
#else
#define assert_unlikely(expr) \
	do { \
		if (SPOOPY_UNLIKELY(!(expr))) { \
			SPOOPY_LOG_ERROR("Assertion failed: %s, file %s, line %d", #expr, __FILE__, __LINE__); \
			abort(); \
		} \
	} while(0)
#endif

#ifdef NDEBUG
#define spoopy_assert(expr) ((void)sizeof(expr))
#else
#define spoopy_assert(expr) \
	do { \
		if (!(expr)) { \
			SPOOPY_LOG_ERROR("Assertion failed: %s, file %s, line %d", #expr, __FILE__, __LINE__); \
			abort(); \
		} \
	} while(0)
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
