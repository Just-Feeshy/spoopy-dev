#ifndef SPOOPY_ASSERT_H
#define SPOOPY_ASSERT_H

#include <spoopy.h>
#include <spoopy_log.h>

#define assert(expr) \
    if (!(expr)) { \
        SPOOPY_LOG_ERROR("Assertion failed: %s, file %s, line %d", #expr, __FILE__, __LINE__); \
        abort(); \
    }

inline void runtime_assert(bool expr, const char* message) {
    SPOOPY_LOG_ERROR("Runtime assertion failed: %s", message);
    assert(expr != NULL);
}

#if defined(__STDC_VERSION__)
    #if __STDC_VERSION__ <= 201710L
        #define static_assert _Static_assert
    #endif
#endif

#endif // SPOOPY_ASSERT_H
