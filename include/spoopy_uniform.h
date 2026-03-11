#pragma once

#include <spoopy.h>
#include <spoopy_types.h>
#include <spoopy_reflect.h>
#include <memory/spoopy_arena.h>

#ifdef __cplusplus
extern "C" {
#endif


typedef enum spoopy_uniform_type {
	SPOOPY_UNIFORM_FLOAT,
	SPOOPY_UNIFORM_VEC2,
	SPOOPY_UNIFORM_VEC3,
	SPOOPY_UNIFORM_VEC4,
	SPOOPY_UNIFORM_INT,
	SPOOPY_UNIFORM_IVEC2,
	SPOOPY_UNIFORM_IVEC3,
	SPOOPY_UNIFORM_IVEC4,
	SPOOPY_UNIFORM_SAMPLER_2D,
	SPOOPY_UNIFORM_SAMPLER_CUBE,
	SPOOPY_UNIFORM_MAT3,
	SPOOPY_UNIFORM_MAT4,
	SPOOPY_UNIFORM_UNKNOWN,
} spoopy_uniform_type_t;

typedef struct spoopy_uniform {
	const char* name;
	spoopy_hash_t hash;
	spoopy_uniform_type_t type;

	union {
		struct {
			uint8_t* data;
			uint16_t offset;
			spoopy_data_type_t data_type;
		} buffer_backed;

		struct {
			uint16_t binding;
			uint16_t paired_binding;
			ShaderSamplerType sampler_type;
		} sampler;
	};
} spoopy_uniform_t;

typedef spoopy_uniform_t spoopy_uniform_object_t;

typedef struct spoopy_uniform_ht_entry {
	const char* key;
	spoopy_hash_t hash;
	spoopy_uniform_t value;
} spoopy_uniform_ht_entry_t;

typedef struct spoopy_uniform_ht {
	size_t count;
	size_t capacity;
	spoopy_uniform_ht_entry_t* entries;
} spoopy_uniform_ht_t;

SPOOPY_VECTOR(spoopy_uniform_vec, spoopy_uniform_t)

SPOOPY_FUNC_CORE spoopy_hash_t spoopy_uniform_hash_string(const char* key);
SPOOPY_FUNC_CORE size_t spoopy_uniform_ht_capacity(size_t item_count);
SPOOPY_FUNC_CORE bool spoopy_uniform_ht_init(spoopy_uniform_ht_t* table, spoopy_mem_arena_t* arena, size_t item_count);
SPOOPY_FUNC_CORE spoopy_uniform_t* spoopy_uniform_ht_get(spoopy_uniform_ht_t* table, const char* key);
SPOOPY_FUNC_CORE bool spoopy_uniform_ht_insert_copy(
	spoopy_uniform_ht_t* table,
	spoopy_mem_arena_t* arena,
	const spoopy_uniform_t* src,
	uint8_t* buffer_data
);

#ifdef __cplusplus
}
#endif
