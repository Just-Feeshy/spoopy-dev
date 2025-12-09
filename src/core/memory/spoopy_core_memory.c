#include <spoopy.h>
#include <memory/spoopy_memory.h>
#include <utils/spoopy_misc_math.h>

void* spoopy_span_alloc(size_t size) {
	const uint32_t header_bits = (uint32_t)(sizeof(spoopy_header_t) * CHAR_BIT);
	assert(header_bits > 1U);
	assert((header_bits - 1U) < (sizeof(size_t) * CHAR_BIT));

	const size_t max_size = ((size_t)1U << (header_bits - 1U)) - 1U;
	assert(size <= max_size);

	size_t alignment = spoopy_align_manually(
		sizeof(size) + sizeof(spoopy_header_t) + sizeof(void*),
		SPOOPY_MAX_ALIGN
	);

	void* raw_mem = spoopy_aligned_alloc(
		alignment,
		size + sizeof(spoopy_header_t)
	);

	{
		spoopy_header_t* s16 = (spoopy_header_t*)raw_mem;
		s16->is_unique = 0;
		s16->unsigned_size = (uint16_t)size;
	}

	uint8_t* s8 = (uint8_t*)raw_mem;
	return (void*)(s8 + sizeof(spoopy_header_t));
}

void spoopy_span_free(void* ptr) {
	if (!ptr) return;

	uint8_t* s8 = (uint8_t*)ptr;
	spoopy_header_t* header = (spoopy_header_t*)(s8 - sizeof(spoopy_header_t));

	size_t size = (size_t)(header->unsigned_size);
	spoopy_heap_free((void*)header);
}
