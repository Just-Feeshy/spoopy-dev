#pragma once

#include <utils/spoopy_misc_math.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SPOOPY_INTERPOLATION(type) typedef struct { \
	type curr; \
	type prev; \
	type _next; \
	bool _need_update; \
} spoopy_interp_##type##_t; \
static inline spoopy_interp_##type##_t spoopy_interp_##type##_init(void) { \
	spoopy_interp_##type##_t interp; \
	interp._need_update = false; \
	interp.curr = (type)0; \
	interp.prev = (type)0; \
	interp._next = (type)0; \
	return interp; \
} \
static inline bool spoopy_interpolate_##type (spoopy_interp_##type##_t* interp, float frac) { \
	if(interp->_need_update) { \
		interp->_next = spoopy_lerp_f32(interp->prev, interp->curr, frac); \
		return true; \
	} \
	return false; \
} \
static inline bool spoopy_interp_##type##_equals(const spoopy_interp_##type##_t interp, type o) { \
	return interp.curr == o; \
} \
static inline void spoopy_interp_##type##_set(spoopy_interp_##type##_t* interp, type value) { \
	interp->curr = value; \
	interp->_next = value; \
	interp->_need_update = true; \
} \
static inline type spoopy_interp_##type##_get(spoopy_interp_##type##_t interp) { \
	return interp.curr; \
}

SPOOPY_INTERPOLATION(float)

#ifdef __cplusplus
}
#endif
