#ifndef SPOOPY_ASSERT_H
#define SPOOPY_ASSERT_H

#include <spoopy_log.h>

#define assert_unlikely(expr) \
    if (SPOOPY_UNLIKELY(!(expr))) { \
        SPOOPY_LOG_ERROR("Assertion failed: %s, file %s, line %d", #expr, __FILE__, __LINE__); \
        abort(); \
    }

#define assert(expr) \
    if (!(expr)) { \
        SPOOPY_LOG_ERROR("Assertion failed: %s, file %s, line %d", #expr, __FILE__, __LINE__); \
        abort(); \
    }

static inline void runtime_assert(bool expr, const char* message) {
    if(!expr) {
        SPOOPY_LOG_ERROR("Runtime assertion failed: %s", message);
        abort();
    }
}

#if defined(__STDC_VERSION__)

#if __STDC_VERSION__ <= 201710L
#define static_assert _Static_assert
#endif

#endif // __STDC_VERSION__ check

#endif // SPOOPY_ASSERT_H
