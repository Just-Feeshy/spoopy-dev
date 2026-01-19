#pragma once

#include <utils/spoopy_compat.h>


// Why is my neovim using regular tabs instead of spaces for this file only?
// Any one else contributing, please fix the tabs for this file, this looks ugly.

#ifndef SPOOPY_THROW_BEHAVIOR
  #if defined(_MSC_VER)
    #include <intrin.h>
    #define SPOOPY_THROW_BEHAVIOR() do { __debugbreak(); abort(); } while (0)
  #elif defined(__has_builtin)
    #if __has_builtin(__builtin_trap)
      #define SPOOPY_THROW_BEHAVIOR() do { __builtin_trap(); } while (0)
    #else
      #define SPOOPY_THROW_BEHAVIOR() do { abort(); } while (0)
    #endif
  #else
    #define SPOOPY_THROW_BEHAVIOR() do { abort(); } while (0)
  #endif
#endif


#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
        LOG_INFO,
        LOG_WARN,
        LOG_ERROR,
        LOG_SUCCESS
} spoopyLogType;

static inline void spoopy_log_internal(spoopyLogType type, const char* file, int line, const char* fmt, ...)
        SPOOPY_ATTR(format(printf, 4, 5))
        SPOOPY_ATTR(nonnull(4));

static inline const char* spoopy_log_prefix(spoopyLogType type) {
        switch (type) {
                case LOG_INFO: return "[SPOOPY INFO]";
                case LOG_WARN: return "[SPOOPY WARN]";
                case LOG_ERROR: return "[SPOOPY ERROR]";
                case LOG_SUCCESS: return "[SPOOPY SUCCESS]";
                default: return "[SPOOPY LOG]";
        }
}

static inline const char* spoopy_log_color(spoopyLogType type) {
        switch (type) {
                case LOG_INFO: return "\033[0m";
                case LOG_WARN: return "\033[1m\033[33m";
                case LOG_ERROR: return "\033[1m\033[31m";
                case LOG_SUCCESS: return "\033[1m\033[32m";
                default: return "\033[0m";
        }
}

static inline void spoopy_log_vinternal(spoopyLogType type,
                                       const char* file,
                                       int line,
                                       const char* fmt,
                                       va_list args)
{
    printf("\033[1m\033[37m%s\033[0m %s[%s:%d] ",
           spoopy_log_prefix(type),
           spoopy_log_color(type),
           file,
           line);

    vprintf(fmt, args);

    printf("\033[0m\n");
}

static inline void spoopy_log_internal(spoopyLogType type, const char* file, int line, const char* fmt, ...) {
	va_list args;
    va_start(args, fmt);
    spoopy_log_vinternal(type, file, line, fmt, args);
    va_end(args);
}

static inline void spoopy_log_throw_internal(spoopyLogType type,
                                             const char* file,
                                             int line,
                                             const char* fmt,
                                             ...)
{

	va_list args;
    va_start(args, fmt);
	spoopy_log_vinternal(type, file, line, fmt, args);
    va_end(args);

    SPOOPY_THROW_BEHAVIOR();
}


#define SPOOPY_LOG_INFO(...)    spoopy_log_internal      (LOG_INFO, __FILE__, __LINE__, __VA_ARGS__)
#define SPOOPY_LOG_WARN(...)    spoopy_log_internal      (LOG_WARN, __FILE__, __LINE__, __VA_ARGS__)
#define SPOOPY_LOG_ERROR(...)   spoopy_log_internal      (LOG_ERROR, __FILE__, __LINE__, __VA_ARGS__)
#define SPOOPY_LOG_THROW(...)   spoopy_log_throw_internal(LOG_ERROR, __FILE__, __LINE__, __VA_ARGS__)
#define SPOOPY_LOG_SUCCESS(...) spoopy_log_internal      (LOG_SUCCESS, __FILE__, __LINE__, __VA_ARGS__)

#define SPOOPY_LOG_ERR_COND(cond) \
	do { \
		if (SPOOPY_UNLIKELY(cond)) { \
			SPOOPY_LOG_ERROR("%s:%d - Condition '%s' failed.", __FILE__, __LINE__, #cond); \
			abort(); \
		} \
	} while(0)

#ifdef __cplusplus
}
#endif
