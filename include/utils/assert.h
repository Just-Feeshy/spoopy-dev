#pragma once

#include <spoopy_log.h>

#define assert_unlikely(expr) \
	do { \
		if (SPOOPY_UNLIKELY(!(expr))) { \
			SPOOPY_LOG_ERROR("Assertion failed: %s, file %s, line %d", #expr, __FILE__, __LINE__); \
			abort(); \
		} \
	} while(0)

#define spoopy_assert(expr) \
	do { \
		if (!(expr)) { \
			SPOOPY_LOG_ERROR("Assertion failed: %s, file %s, line %d", #expr, __FILE__, __LINE__); \
			abort(); \
		} \
	} while(0)

#ifndef assert
#define assert(expr) spoopy_assert(expr)
#endif

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
