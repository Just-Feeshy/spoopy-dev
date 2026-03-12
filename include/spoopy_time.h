#pragma once

#include <spoopy.h>

#ifdef __cplusplus
extern "C" {
#endif

extern const size_t SPOOPY_TIME_RESOLUTION;

SPOOPY_FUNC_CORE size_t spoopy_time_get(void);

#ifdef __cplusplus
}
#endif
