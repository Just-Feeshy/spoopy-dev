#include <spoopy.h>
#include <limits.h>
#include <memory/spoopy_memory.h>
#include <utils/spoopy_misc_math.h>

static size_t spoopy_static_max_payload(void) {
	const uint32_t header_bits = (uint32_t)(sizeof(spoopy_header_t) * CHAR_BIT);
	assert(header_bits > 1U);
	assert((header_bits - 1U) < (sizeof(size_t) * CHAR_BIT));
	return ((size_t)1U << (header_bits - 1U)) - 1U;
}

void* spoopy_static_alloc(size_t size) {
	const size_t max_size = spoopy_static_max_payload();
	assert(size <= max_size);

const size_t alignment = SPOOPY_MAX_ALIGN;
const size_t allocation_size = spoopy_align_manually(
	size + sizeof(spoopy_header_t),
	alignment
);

void* raw_mem = spoopy_aligned_alloc(
	alignment,
	allocation_size
);

if (!raw_mem) {
	return NULL;
}

	{
		spoopy_header_t* mem = (spoopy_header_t*)raw_mem;
		mem->is_unique = 0;
		mem->unsigned_size = (uint16_t)size;
	}

	uint8_t* s8 = (uint8_t*)raw_mem;
	return (void*)(s8 + sizeof(spoopy_header_t));
}

void spoopy_static_free(void* ptr) {
	if (!ptr) return;

	uint8_t* s8 = (uint8_t*)ptr;
	spoopy_header_t* header = (spoopy_header_t*)(s8 - sizeof(spoopy_header_t));

	size_t size = (size_t)(header->unsigned_size);
	spoopy_heap_free((void*)header);
}

void* spoopy_static_realloc(void* ptr, size_t size) {
	if (!ptr) {
		return size == 0 ? NULL : spoopy_static_alloc(size);
	}

	if (size == 0) {
		spoopy_static_free(ptr);
		return NULL;
	}

	uint8_t* payload = (uint8_t*)ptr;
	spoopy_header_t* header = (spoopy_header_t*)(payload - sizeof(spoopy_header_t));
	const size_t old_size = (size_t)(header->unsigned_size);

	if (size == old_size) {
		return ptr;
	}

	const size_t total_allocation = size + sizeof(spoopy_header_t);
	void* resized = spoopy_heap_realloc((void*)header, total_allocation);
	if (!resized) {
		return NULL;
	}

	spoopy_header_t* new_header = (spoopy_header_t*)resized;
	new_header->is_unique = 0;
	new_header->unsigned_size = (uint16_t)size;

	uint8_t* new_payload = (uint8_t*)resized;
	return (void*)(new_payload + sizeof(spoopy_header_t));
}
