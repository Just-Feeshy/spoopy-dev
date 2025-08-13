#ifndef SPOOPY_LOG_H
#define SPOOPY_LOG_H

#include <utils/spoopy_compat.h>


// Why is my neovim using regular tabs instead of spaces for this file only?
// Any one else contributing, please fix the tabs for this file, this looks ugly.

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
        LOG_INFO,
        LOG_WARN,
        LOG_ERROR,
        LOG_SUCCESS
} spoopyLogType;

static inline void spoopy_log(spoopyLogType type, const char* fmt, ...)
        SPOOPY_ATTR(format(printf, 2, 3))
        SPOOPY_ATTR(nonnull(2));

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

static inline void spoopy_log(spoopyLogType type, const char* fmt, ...) {
        printf("\033[1m\033[37m%s\033[0m %s", spoopy_log_prefix(type), spoopy_log_color(type));

        va_list args;
        va_start(args, fmt);
        vprintf(fmt, args);
        va_end(args);

        printf("\033[0m\n");
}

#define SPOOPY_LOG_INFO(...)    spoopy_log(LOG_INFO, __VA_ARGS__)
#define SPOOPY_LOG_WARN(...)    spoopy_log(LOG_WARN, __VA_ARGS__)
#define SPOOPY_LOG_ERROR(...)   spoopy_log(LOG_ERROR, __VA_ARGS__)
#define SPOOPY_LOG_SUCCESS(...) spoopy_log(LOG_SUCCESS, __VA_ARGS__)

#ifdef __cplusplus
}
#endif

#endif // SPOOPY_LOG_H
