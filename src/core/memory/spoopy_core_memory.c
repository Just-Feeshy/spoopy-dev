#include <spoopy.h>
#include <limits.h>
#include <memory/spoopy_memory.h>
#include <utils/spoopy_misc_math.h>
#include <utils/assert.h>

static size_t spoopy_static_max_payload(void) {
	const uint32_t header_bits = (uint32_t)(sizeof(spoopy_header_t) * CHAR_BIT);
	assert(header_bits > 1U);
	assert((header_bits - 1U) < (sizeof(size_t) * CHAR_BIT));
	return ((size_t)1U << (header_bits - 1U)) - 1U;
}

void* spoopy_static_alloc(size_t size) {
	const size_t max_size = spoopy_static_max_payload();
	assert(size <= max_size);

	const size_t allocation = size + sizeof(spoopy_header_t);
	const size_t alignment = spoopy_align_manually(
		size + sizeof(spoopy_header_t) + sizeof(void*),
		SPOOPY_MAX_ALIGN
	);

	spoopy_header_t* s = spoopy_aligned_alloc(allocation, alignment);
	assert(s != NULL);

	s->is_unique = 0;
	s->unsigned_size = (uint16_t)size;
	return s + 1;
}

void spoopy_static_free(void* ptr) {
	assert(ptr != NULL);

	uint8_t* s8 = (uint8_t*)ptr;
	s8 -= sizeof(spoopy_header_t);
	spoopy_heap_free(s8);
}

void* spoopy_static_realloc(void* ptr, size_t size) {
	if (!ptr) {
		return spoopy_static_alloc(size);
	}

	uint8_t* s8 = (uint8_t*)ptr;
	s8 -= sizeof(spoopy_header_t);

	if(size == 0) {
		spoopy_heap_free(s8);
		return NULL;
	}

	spoopy_header_t* header = (spoopy_header_t*)ptr - 1;
	if((uint16_t)size <= header->unsigned_size) {
		return ptr;
	}

	void* new_ptr = spoopy_static_alloc(size);
	memcpy(new_ptr, ptr, size);
	spoopy_heap_free(s8);

	return new_ptr;
}
