#include "../spoopy_system_info.h"
#include <spoopy.h>
#include <memory/spoopy_memory.h>
#include <memory/spoopy_arena.h>
#include <utils/spoopy_misc_math.h>
#include <spoopy_log.h>
#include <utils/assert.h>

#ifdef _MSC_VER
#include <malloc.h>
#endif

#include <mimalloc.h>


// TODO (Framework): Have support for users to define their own memory allocators

void spoopy_heap_free(void* ptr) {
    mi_free(ptr);
}

void* spoopy_heap_alloc(size_t size) {
    return spoopy_aligned_alloc(SPOOPY_MAX_ALIGN, size, NULL);
}

void* spoopy_aligned_alloc(size_t alignment, size_t size, void* user_data) {
    (void)user_data;
    alignment = spoopy_max(alignment, SPOOPY_MAX_ALIGN);
    return mi_calloc_aligned(1, size, alignment);
}

void* spoopy_heap_realloc(void* ptr, size_t size) {
    assert(size > 0);
    return mi_realloc_aligned(ptr, size, SPOOPY_MAX_ALIGN);
}


static inline spoopy_mem_arena_page_t* arena_active_page(spoopy_mem_arena_t* arena) {
	spoopy_mem_arena_page_t* page = arena->pages.end_page;

	if(page) {
		assume(page->next == NULL);
	}

	return page;
}

// So I solved this problem in Leetcode before I was designing this
// It's true what they say. GRIND LEETCODE!!
// https://leetcode.com/problems/delete-node-in-a-linked-list/
static inline void	delete_page(spoopy_mem_arena_page_t* page) {
	spoopy_mem_arena_page_t* p = page->next;
	memcpy(page, p, sizeof(spoopy_mem_arena_page_t) + p->size);
	mi_free(p);
}

static inline bool arena_try_delete_unused_page(spoopy_mem_arena_t* arena, spoopy_mem_arena_page_t* page, size_t page_offset) {
	if(page_offset == 0 && page != arena->pages.end_page) {
		arena->total_allocated -= page->size;
		delete_page(page);
		return true;
	}

	return false;
}

static void append_page(spoopy_mem_arena_t* arena, spoopy_mem_arena_page_t* page) {
	page->next = NULL;
	if(arena->pages.end_page == NULL) {
		assert(arena->pages.begin_page == NULL);
		arena->pages.begin_page = page;
	}else {
		assert(arena->pages.begin_page != NULL);
		arena->pages.end_page->next = page;
	}

	arena->pages.end_page = page;
}

static spoopy_mem_arena_page_t* shift_page(spoopy_mem_arena_t* arena) {
	spoopy_mem_arena_page_t* p = arena->pages.begin_page;

	if(p != NULL) {
		spoopy_mem_arena_page_t* temp = p->next;
		mi_free(p);
		arena->pages.begin_page = temp;
	}

	return arena->pages.begin_page;
}

static spoopy_mem_arena_page_t* arena_new_page(spoopy_mem_arena_t* arena, size_t min_size) {
	size_t alloc_size = spoopy_ceil_pow2_size(min_size + sizeof(spoopy_mem_arena_page_t));
	alloc_size = spoopy_max(alloc_size, SPOOPY_ARENA_MIN_SIZE);

	size_t page_size = alloc_size - sizeof(spoopy_mem_arena_page_t);
	spoopy_mem_arena_page_t* p = mi_calloc_aligned(1, alloc_size, SPOOPY_MAX_ALIGN);
	p->size = page_size;
	arena->pages.begin_page = p;
	append_page(arena, p);
	arena->page_offset = 0;
	arena->total_allocated += page_size;
	return p;
}

static void* arena_alloc(spoopy_mem_arena_t* arena, size_t size, size_t align, bool free_unused_page) {
	spoopy_mem_arena_page_t* page = arena_active_page(arena);

	if(SPOOPY_UNLIKELY(!page)) {
		page = arena_new_page(arena, size);
	}

	size_t page_ofs = arena->page_offset;
	size_t required, alignofs;

	for(;;) {
		size_t available = page->size - page_ofs;
		alignofs = (align - (uintptr_t)(page->data + page_ofs)) & (align - 1);
		required = alignofs + size;

		if(available < required) {
			spoopy_mem_arena_page_t* prev_page = page;

			size_t new_page_size = (arena->total_used >> 1) + required;
			page = arena_new_page(arena, new_page_size);
			assert(arena->page_offset == 0);

			if(free_unused_page) {
				arena_try_delete_unused_page(arena, prev_page, page_ofs);
			}

			page_ofs = 0;
			continue;
		}

		break;
	}

	void* p = page->data + page_ofs + alignofs;
	arena->total_used += required;
	arena->page_offset += required;
	assert(arena->page_offset <= page->size);
	assert(((uintptr_t)p & (align - 1)) == 0);
	return p;
}

void spoopy_arena_init(spoopy_mem_arena_t* arena, size_t min_size) {
	*arena = (spoopy_mem_arena_t) { 0 };
	if(min_size > 0) {
		arena_new_page(arena, min_size);
	}
}

void spoopy_arena_deinit(spoopy_mem_arena_t *arena) {
	SPOOPY_ATTR_UNUSED spoopy_mem_arena_page_t* p;
	while((p = shift_page(arena))) {
#ifdef SPOOPY_BUILD_DEBUG
		SPOOPY_LOG_INFO("Pointer with size (%lu) has be deallocated", p->size);
#endif
	};
}

void spoopy_arena_reset(spoopy_mem_arena_t *arena) {
	size_t used = arena->total_used;
	arena->total_used = 0;
	arena->page_offset = 0;

	if(arena->pages.begin_page->next) {
		spoopy_mem_arena_page_t* p;
		spoopy_arena_deinit(arena);
		arena->total_allocated = 0;
		p = arena_new_page(arena, used);
		assert(p == arena->pages.end_page);
	}

	assert(arena->pages.begin_page != NULL);
	assert(arena->pages.begin_page == arena->pages.end_page);
}

void* spoopy_arena_alloc(spoopy_mem_arena_t* arena, size_t size) {
	return arena_alloc(arena, size, SPOOPY_MAX_ALIGN, true);
}

bool spoopy_arena_free(spoopy_mem_arena_t* restrict arena, void* restrict p, size_t old_size) {
	spoopy_mem_arena_page_t* page = arena_active_page(arena);

	if(SPOOPY_UNLIKELY(!page)) {
		assert(p == NULL);
		assert(old_size == 0);
		return false;
	}

	if(page->data + arena->page_offset - old_size == p) {
		assert(arena->page_offset >= old_size);
		arena->page_offset -= old_size;
		arena->total_used -= old_size;
		return true;
	}

	return false;
}
