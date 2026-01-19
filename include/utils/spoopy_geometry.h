#pragma once

#include "spoopy_misc_math.h"

#include <spoopy.h>

#ifdef __cplusplus
extern "C" {
#endif


/*
 * ============================= Spoopy Core Types =============================
 * All core types used by the Spoopy Environment and Renderer API are
 * defined in this header, it's more or less a nice one stop shop for
 * all the basic structures and types used throughout the literal entire
 * codebase
 * =============================================================================
 */

#define SPOOPY_VEC_AND_REC_TYPES(type) \
	typedef union spoopy_vec2_##type { \
		struct { \
			type x, y; \
		}; \
		struct { \
			type w, h; \
		}; \
		struct { \
			type r, g; \
		}; \
		type data[2]; \
	} spoopy_vec2_##type##_t; \
	typedef union spoopy_vec3_##type { \
		struct { \
			type x, y, z; \
		}; \
		type data[3]; \
	} spoopy_vec3_##type##_t; \
	typedef union spoopy_vec4_##type { \
		struct { \
			type x, y, z, w; \
		}; \
		type data[4]; \
	} spoopy_vec4_##type##_t; \
	typedef struct spoopy_rec_##type { \
		spoopy_vec2_##type##_t point; \
		spoopy_vec2_##type##_t size; \
	} spoopy_rec_##type##_t;

#define SPOOPY_VEC_FUNCTIONS(type, num) \
static inline void spoopy_vec##num##_##type##_mul(spoopy_vec##num##_##type##_t* vec, type v) { \
    for(int i=0; i<(num); i++) { \
        vec->data[i] *= v; \
    } \
} \
static inline void spoopy_vec##num##_##type##_div(spoopy_vec##num##_##type##_t* vec, type v) { \
    for(int i=0; i<(num); i++) { \
        vec->data[i] /= v; \
    } \
} \
static inline void spoopy_vec##num##_##type##_sub(spoopy_vec##num##_##type##_t* vec, spoopy_vec##num##_##type##_t sub_vec) { \
	for(int i=0; i<(num); i++) { \
		vec->data[i] -= sub_vec.data[i]; \
	} \
} \
static inline void spoopy_vec##num##_##type##_add(spoopy_vec##num##_##type##_t* vec, spoopy_vec##num##_##type##_t add_vec) { \
	for(int i=0; i<(num); i++) { \
		vec->data[i] += add_vec.data[i]; \
	} \
} \
static inline void spoopy_vec##num##_##type##_clamp(spoopy_vec##num##_##type##_t* v, spoopy_vec##num##_##type##_t lo, spoopy_vec##num##_##type##_t hi) { \
	for(int i=0; i<(num); i++) { \
		v->data[i] = spoopy_clamp(v->data[i], lo.data[i], hi.data[i]); \
	} \
}

#define SPOOPY_REC_FUNCTIONS(type) \
static inline bool spoopy_rec_##type##_equal(spoopy_rec_##type##_t rec1, spoopy_rec_##type##_t rec2) { \
	return rec1.point.x == rec2.point.x \
		&& rec1.point.y == rec2.point.y \
		&& rec1.size.w  == rec2.size.w  \
		&& rec1.size.h  == rec2.size.h; \
}

SPOOPY_VEC_AND_REC_TYPES(float)
SPOOPY_VEC_AND_REC_TYPES(int)

SPOOPY_VEC_FUNCTIONS(float, 2)
SPOOPY_VEC_FUNCTIONS(float, 3)
SPOOPY_VEC_FUNCTIONS(float, 4)
SPOOPY_VEC_FUNCTIONS(int, 2)
SPOOPY_VEC_FUNCTIONS(int, 3)
SPOOPY_VEC_FUNCTIONS(int, 4)

SPOOPY_REC_FUNCTIONS(float)
SPOOPY_REC_FUNCTIONS(int)


#ifdef __cplusplus
}
#endif
