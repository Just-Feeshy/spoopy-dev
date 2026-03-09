#pragma once

#include <spoopy.h>
#include <spoopy_types.h>

#ifdef __cplusplus
extern "C" {
#endif


typedef struct spoopy_uniform {
	const char* name;
	spoopy_hash_t hash;
} spoopy_uniform_t;

SPOOPY_VECTOR(spoopy_uniform_vec, spoopy_uniform_t)

#ifdef __cplusplus
}
#endif
