#pragma once

#include <spoopy.h>
#include <memory/spoopy_memory.h>

// I've never made an arena before in C, shocker, so I want to learn from reading:
// https://github.com/taisei-project/taisei/blob/master/src/memory/arena.h

#ifdef __cplusplus
extern "C" {
#endif

typedef struct spoopy_mem_arena_page {
	struct spoopy_mem_arena_page* next;
	size_t size;
	SPOOPY_ALIGN_AS(SPOOPY_MAX_ALIGN) char data[];
} spoopy_mem_arena_page_t;

typedef struct spoopy_mem_arena {
	struct {
		spoopy_mem_arena_page_t* begin_page;
		spoopy_mem_arena_page_t* end_page;
	} pages;

	size_t page_offset;
	size_t total_used;
	size_t total_allocated;
} spoopy_mem_arena_t;

void spoopy_arena_init(spoopy_mem_arena_t* arena, size_t min_size);
void spoopy_arena_deinit(spoopy_mem_arena_t* arena);
void spoopy_arena_reset(spoopy_mem_arena_t* arena);
void* spoopy_arena_alloc(spoopy_mem_arena_t* arena, size_t size);
bool spoopy_arena_free(spoopy_mem_arena_t* SPOOPY_RESTRICT arena, void* SPOOPY_RESTRICT p, size_t old_size);

static inline void* spoopy_arena_memdup(spoopy_mem_arena_t* arena, const void* buf, size_t size) {
	return memcpy(spoopy_arena_alloc(arena, size), buf, size);
}

static inline char* spoopy_arena_strdup(spoopy_mem_arena_t* arena, const char* src) {
	return (char*)spoopy_arena_memdup(arena, src, strlen(src) + 1);
}

#ifdef __cplusplus
}
#endif
