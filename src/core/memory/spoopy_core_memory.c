#include <spoopy.h>
#include <memory/spoopy_memory.h>
#include <utils/assert.h>

static size_t spoopy_static_max_payload(void) {
	const uint32_t header_bits = (uint32_t)(sizeof(spoopy_header_t) * CHAR_BIT);
	assert(header_bits > 1U);
	assert((header_bits - 1U) < (sizeof(size_t) * CHAR_BIT));
	return ((size_t)1U << (header_bits - 1U)) - 1U;
}

spoopy_static_block_t* spoopy_static_alloc(size_t size) {
	const size_t max_size = spoopy_static_max_payload();
	assert(size <= max_size);

	const size_t allocation = size + sizeof(spoopy_header_t);
	const size_t alignment = spoopy_align_bound(
		allocation /*+ sizeof(void*)*/,
		SPOOPY_MAX_ALIGN
	);

	spoopy_static_block_t* s = spoopy_aligned_alloc(alignment, allocation, NULL);
	assert(s);

	s->header.is_unique = true;
	s->header.unsigned_size = (uint16_t)size;
	return s;
}

void spoopy_static_free(void* ptr) {
	if(!ptr) {
		return;
	}

	spoopy_heap_free(spoopy_static_block_from_payload(ptr));
}

spoopy_static_block_t* spoopy_static_realloc(void* ptr, size_t size) {
	if(!ptr) {
		return spoopy_static_alloc(size);
	}

	if(size == 0) {
		spoopy_static_free(ptr);
		return NULL;
	}

	spoopy_static_block_t* new_block = spoopy_static_alloc(size);
	assert(new_block);

	spoopy_static_block_t* old_block = spoopy_static_block_from_payload(ptr);
	const size_t old_size = (size_t)old_block->header.unsigned_size;
	const size_t copy_size = spoopy_min(old_size, size);

	memcpy(new_block->payload, old_block->payload, copy_size);
	spoopy_static_free(ptr);
	return new_block;
}
