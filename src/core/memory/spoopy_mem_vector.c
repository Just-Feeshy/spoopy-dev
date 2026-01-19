#include <memory/spoopy_mem_vector.h>
#include <utils/assert.h>

spoopy_mem_vector_t spoopy_mem_vector_create(size_t initial_capacity, spoopy_growth_callback_t growth_callback) {
	const size_t extra_size = initial_capacity / (sizeof(size_t) * 8U) + 1U;

	spoopy_mem_vector_t vector = { 0 };
	vector.buffer = NULL;
	vector.length = 0;
	vector.element_size = -1;
	vector.growth_callback = growth_callback;
	vector.chunk_capacity = SPOOPY_FLEX_ALLOC(size_t, extra_size, spoopy_heap);
	vector.chunk_count = extra_size;

	return vector;
}

void spoopy_mem_vector_push_back(spoopy_mem_vector_t* vector, const void* element, const size_t element_size) {
	assert(vector != NULL && element != NULL
		&& (element_size == -1 || element_size == vector->element_size)
	);

	if(vector->element_size == -1) {
		vector->element_size = (int16_t)element_size;
	}

	vector->buffer = spoopy_heap_realloc(
		vector->buffer,
		(vector->length + 1U) * (size_t)(vector->element_size)
	);

	const size_t index = vector->length;
	const size_t chunk_index = index / (sizeof(size_t) * 8U);
	const size_t bit_index = index % (sizeof(size_t) * 8U);

	vector->chunk_capacity[chunk_index] |= ((size_t)1U << bit_index);
	vector->buffer[index * (size_t)(vector->element_size)] = *(const uint8_t*)element;
	vector->length += 1U;

	if(vector->length > chunk_count * (sizeof(size_t) * 8U)) {
		const size_t new_chunk_count = chunk_count + 1U;
		const size_t extra_size = new_chunk_count - chunk_count;

		vector->chunk_capacity = spoopy_heap_realloc(
			vector->chunk_capacity,
			new_chunk_count * sizeof(size_t)
		);

		memset(
			(uint8_t*)vector->chunk_capacity + chunk_count * sizeof(size_t),
			0,
			extra_size * sizeof(size_t)
		);

		vector->chunk_count = new_chunk_count;
	}
}
