#pragma once

#include <spoopy.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef size_t (*spoopy_growth_callback_t)(void* vector, size_t new_size);

typedef struct spoopy_mem_vector {
	void* buffer;
	size_t length;
	int16_t element_size;
	spoopy_growth_callback_t growth_callback;

	size_t chunk_count;
	size_t* chunk_capacity;
} spoopy_mem_vector_t;

SPOOPY_FUNC_CORE spoopy_mem_vector_t spoopy_mem_vector_create(size_t initial_capacity, spoopy_growth_callback_t growth_callback);
SPOOPY_FUNC_CORE void spoopy_mem_vector_push_back(spoopy_mem_vector_t* vector, const void* element, const size_t element_size);

#ifdef __cplusplus
}
#endif
