#pragma once

#include <tinystl/vector.h>
#include <tinystl/unordered_map.h>
#include <bx/string.h>
#include <tinystl/string.h>

#include <spoopy.h>
#include <memory/spoopy_memory.h>

struct spoopy_allocator {
	static void* static_allocate(size_t bytes) {
		return spoopy_heap_alloc(bytes);
	}

	static void static_deallocate(void* ptr, size_t /*bytes*/) {
		spoopy_heap_free(ptr);
	}
};

typedef tinystl::basic_string<spoopy_allocator> tiny_string;
