#pragma once

#include <spoopy.h>
#include <memory/spoopy_memory.h>

// Generic vector container for any datatype
// I will say.. the vector was one of the greatest
// components of C++'s STL

#define SPOOPY_VECTOR(type_name, element_type) \
	SPOOPY_VECTOR_LIMIT(type_name, element_type, size_t)

#define SPOOPY_VECTOR_LIMIT(type_name, element_type, l_type) \
typedef struct type_name { \
	l_type size; \
	l_type capacity; \
	element_type* data; \
} type_name##_t; \
static inline void type_name##_init(type_name##_t* vector, l_type pool_size) { \
	if(!vector) return; \
	const l_type real_pool_size = sizeof(element_type) * pool_size; \
	vector->data = (element_type*)spoopy_heap_alloc(real_pool_size); \
	if(!vector->data) { \
		vector->capacity = 0; \
		vector->size = 0; \
		return; \
	} \
	vector->capacity = 0; \
	vector->size = pool_size; \
	memset(vector->data, 0, real_pool_size); \
} \
static inline void type_name##_destroy(type_name##_t* vector) { \
	if(vector && vector->data) { \
		spoopy_heap_free(vector->data); \
		vector->data = NULL; \
		vector->size = 0; \
		vector->capacity = 0; \
	} \
} \
static inline void type_name##_resize(type_name##_t* vector, l_type new_size) { \
	if(!vector) return; \
	const l_type real_new_size = sizeof(element_type) * new_size; \
	element_type* new_data = (element_type*)spoopy_heap_realloc(vector->data, real_new_size); \
	if(!new_data) return; \
	vector->data = new_data; \
	if(new_size > vector->size) { \
		const l_type old_size = sizeof(element_type) * vector->size; \
		memset((uint8_t*)vector->data + old_size, 0, real_new_size - old_size); \
	} \
	vector->size = new_size; \
} \
static inline void type_name##_compact(type_name##_t* vector) { \
	if(!vector) return; \
	if(vector->capacity >= vector->size) return; \
	type_name##_resize(vector, vector->capacity); \
} \
static inline void type_name##_qsort( \
		type_name##_t* vector, \
		int (*compar)(const void*, const void*)) { \
	if(!vector || !compar) return; \
	qsort(vector->data, vector->capacity, sizeof(element_type), compar); \
} \
static inline void type_name##_add( \
		type_name##_t* vector, \
		element_type value) { \
	if(!vector) return; \
	for(l_type i=0; i<vector->capacity; ++i) { \
		if(memcmp(&vector->data[i], &value, sizeof(element_type)) == 0) { \
			return; \
		} \
	} \
	if(vector->capacity >= vector->size) { \
		const l_type new_size = 8; \
		type_name##_resize(vector, new_size); \
	} \
	vector->data[vector->capacity] = value; \
	vector->capacity++; \
}
