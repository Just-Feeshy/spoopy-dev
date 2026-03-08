#pragma once

#include "spoopy_misc_math.h"

#include <spoopy.h>
#include <memory/spoopy_memory.h>

#ifdef __cplusplus
extern "C" {
#endif

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
typedef struct spoopy_vec##num##_vec_##type { \
	size_t size; \
	size_t capacity; \
	spoopy_vec##num##_##type##_t vecs[]; \
} spoopy_vec##num##_vec_##type##_t; \
static inline spoopy_vec##num##_vec_##type##_t* spoopy_vec##num##_vec_##type##_init(size_t pool_size) { \
	const size_t real_pool_size = sizeof(spoopy_vec##num##_##type##_t) * pool_size; \
	spoopy_vec##num##_vec_##type##_t* vector = SPOOPY_FLEX_ALLOC( \
		spoopy_vec##num##_vec_##type##_t, \
		real_pool_size, \
		spoopy_heap \
	); \
	vector->capacity = 0; \
	vector->size = pool_size; \
	memset(vector->vecs, 0, real_pool_size); \
	return vector; \
} \
static inline void spoopy_vec##num##_vec_##type##_destroy(spoopy_vec##num##_vec_##type##_t* vector) { \
	if(vector) { \
		spoopy_heap_free(vector); \
	} \
} \
static inline spoopy_vec##num##_vec_##type##_t* spoopy_vec##num##_vec_##type##_resize(spoopy_vec##num##_vec_##type##_t* vector, size_t new_size) { \
	const size_t real_new_size = sizeof(spoopy_vec##num##_##type##_t) * new_size; \
	spoopy_vec##num##_vec_##type##_t* new_vector = spoopy_heap_realloc( \
		vector, \
		sizeof(spoopy_vec##num##_vec_##type##_t) + real_new_size \
	); \
	if(new_vector) { \
		if(new_size > new_vector->size) { \
			const size_t old_size = sizeof(spoopy_vec##num##_##type##_t) * new_vector->size; \
			memset((uint8_t*)new_vector->vecs + old_size, 0, real_new_size - old_size); \
		} \
		new_vector->size = new_size; \
	} \
	return new_vector; \
} \
static inline void spoopy_vec##num##_vec_##type##_compact(spoopy_vec##num##_vec_##type##_t** vector) { \
	if(!vector || !*vector) return; \
	if((*vector)->capacity >= (*vector)->size) return; \
	spoopy_vec##num##_vec_##type##_t* compacted = spoopy_vec##num##_vec_##type##_resize(*vector, (*vector)->capacity); \
	if(compacted) { \
		*vector = compacted; \
	} \
} \
static inline void spoopy_vec##num##_vec_##type##_qsort( \
		spoopy_vec##num##_vec_##type##_t* vector, \
		int (*compar)(const void*, const void*)) { \
	if(!vector || !compar) return; \
	qsort(vector->vecs, vector->capacity, sizeof(spoopy_vec##num##_##type##_t), compar); \
} \
static inline void spoopy_vec##num##_vec_##type##_add( \
		spoopy_vec##num##_vec_##type##_t** vector, \
		spoopy_vec##num##_##type##_t value) { \
	for(size_t i=0; i<(*vector)->capacity; ++i) { \
		if(memcmp((*vector)->vecs[i].data, value.data, (num) * sizeof(type)) == 0) { \
			return; \
		} \
	} \
	if((*vector)->capacity >= (*vector)->size) { \
		const size_t new_size = 8; \
		spoopy_vec##num##_vec_##type##_t* resized = spoopy_vec##num##_vec_##type##_resize(*vector, new_size); \
		if(!resized) return; \
		*vector = resized; \
	} \
	(*vector)->vecs[(*vector)->capacity] = value; \
	(*vector)->capacity++; \
} \
static inline void spoopy_vec##num##_vec_##type##_add_if_bounded( \
		spoopy_vec##num##_vec_##type##_t** vector, \
		spoopy_vec##num##_##type##_t value, \
		spoopy_vec##num##_##type##_t min, \
		spoopy_vec##num##_##type##_t max) { \
	if(!vector || !*vector) return; \
	bool bound_check = false; \
	for(uint8_t i=0; i<(num); ++i) { \
		bound_check |= ((value.data[i] > max.data[i] && max.data[i] > 0) \
                    ||  (value.data[i] < min.data[i])); \
		if(bound_check) return; \
	} \
	spoopy_vec##num##_vec_##type##_add(vector, value); \
} \
static inline void spoopy_vec##num##_##type##_mul(spoopy_vec##num##_##type##_t* vec, type v) { \
    for(uint8_t i=0; i<(num); ++i) { \
        vec->data[i] *= v; \
    } \
} \
static inline void spoopy_vec##num##_##type##_div(spoopy_vec##num##_##type##_t* vec, type v) { \
    for(uint8_t i=0; i<(num); ++i) { \
        vec->data[i] /= v; \
    } \
} \
static inline void spoopy_vec##num##_##type##_sub(spoopy_vec##num##_##type##_t* vec, spoopy_vec##num##_##type##_t sub_vec) { \
	for(uint8_t i=0; i<(num); ++i) { \
		vec->data[i] -= sub_vec.data[i]; \
	} \
} \
static inline void spoopy_vec##num##_##type##_add(spoopy_vec##num##_##type##_t* vec, spoopy_vec##num##_##type##_t add_vec) { \
	for(uint8_t i=0; i<(num); ++i) { \
		vec->data[i] += add_vec.data[i]; \
	} \
} \
static inline void spoopy_vec##num##_##type##_clamp(spoopy_vec##num##_##type##_t* v, spoopy_vec##num##_##type##_t lo, spoopy_vec##num##_##type##_t hi) { \
	for(uint8_t i=0; i<(num); ++i) { \
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
