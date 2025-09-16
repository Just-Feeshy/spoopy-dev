#pragma once

#include <memory/spoopy_memory.h>
#include <utils/assert.h>

#define DA_MALLOC spoopy_heap_alloc
#define DA_REALLOC spoopy_heap_realloc
#define DA_FREE spoopy_heap_free
#define DA_ASSERT(expr) spoopy_assert(expr)

#define DA_IMPLEMENTATION
#include <dynamic_array.h>

#define da_foreach_elem(arr, elem_var, type, body) do { \
    size_t _da_len = DA_LENGTH(arr); \
    type *_da_data = (type*)da_data(arr); \
    for (size_t _da_i = 0; _da_i < _da_len; _da_i++) { \
        type elem_var = _da_data[_da_i]; \
        body \
    } \
} while(0)
