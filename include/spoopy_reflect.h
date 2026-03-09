#pragma once

#include <spoopy.h>
#include <spoopy_types.h>

// "Borrowed" form Taisei Project:
// Source: https://github.com/taisei-project/taisei/blob/master/src/renderer/common/shaderlib/reflect.h
// It's very useful for slang too


#ifdef __cplusplus
extern "C" {
#endif

typedef enum spoopy_shader_base_type {
	SPOOPY_SHADER_BASE_TYPE_UNKNOWN = 0,
	SPOOPY_SHADER_BASE_TYPE_VOID = 1,
	SPOOPY_SHADER_BASE_TYPE_BOOLEAN = 2,
	SPOOPY_SHADER_BASE_TYPE_INT8 = 3,
	SPOOPY_SHADER_BASE_TYPE_UINT8 = 4,
	SPOOPY_SHADER_BASE_TYPE_INT16 = 5,
	SPOOPY_SHADER_BASE_TYPE_UINT16 = 6,
	SPOOPY_SHADER_BASE_TYPE_INT32 = 7,
	SPOOPY_SHADER_BASE_TYPE_UINT32 = 8,
	SPOOPY_SHADER_BASE_TYPE_INT64 = 9,
	SPOOPY_SHADER_BASE_TYPE_UINT64 = 10,
	SPOOPY_SHADER_BASE_TYPE_ATOMIC_COUNTER = 11,
	SPOOPY_SHADER_BASE_TYPE_FP16 = 12,
	SPOOPY_SHADER_BASE_TYPE_FP32 = 13,
	SPOOPY_SHADER_BASE_TYPE_FP64 = 14,
	SPOOPY_SHADER_BASE_TYPE_STRUCT = 15,
	SPOOPY_SHADER_BASE_TYPE_IMAGE = 16,
	SPOOPY_SHADER_BASE_TYPE_SAMPLED_IMAGE = 17,
	SPOOPY_SHADER_BASE_TYPE_SAMPLER = 18,
	SPOOPY_SHADER_BASE_TYPE_ACCELERATION_STRUCTURE = 19,
} spoopy_shader_base_type_t;

typedef struct spoopy_data_type {
	spoopy_shader_base_type_t base_type;
	uint16_t vector_size;
	uint16_t array_size;  // Flatten approach is the simplest
	uint16_t array_stride;
	uint16_t matrix_columns;
	uint16_t matrix_stride;
} spoopy_data_type_t;

typedef struct spoopy_shader_struct_field {
	const char* name;
	uint16_t offset;
	spoopy_data_type_t type;
} spoopy_shader_struct_field_t;

typedef struct spoopy_shader_block {
	const char *name;
	uint16_t set;
	uint16_t binding;
	uint16_t size;
	uint16_t num_fields;
	spoopy_shader_struct_field_t* fields;
} spoopy_shader_block_t;

typedef enum spoopy_shader_sampler_dimension {
	SPOOPY_SHADER_SAMPLER_DIM_UNKNOWN,
	SPOOPY_SHADER_SAMPLER_DIM_1D,
	SPOOPY_SHADER_SAMPLER_DIM_2D,
	SPOOPY_SHADER_SAMPLER_DIM_3D,
	SPOOPY_SHADER_SAMPLER_DIM_CUBE,
	SPOOPY_SHADER_SAMPLER_DIM_BUFFER,
} spoopy_shader_sampler_dimension_t;


typedef uint16_t ShaderSamplerTypeFlags;

#define SHADER_SAMPLER_DEPTH              (ShaderSamplerTypeFlags)(1 << 0)
#define SHADER_SAMPLER_ARRAYED            (ShaderSamplerTypeFlags)(1 << 1)
#define SHADER_SAMPLER_MULTISAMPLED       (ShaderSamplerTypeFlags)(1 << 2)

typedef struct ShaderSamplerType {
	spoopy_shader_sampler_dimension_t dim : 16;
	ShaderSamplerTypeFlags flags;
} ShaderSamplerType;

typedef struct spoopy_shader_sampler {
	const char *name;
	ShaderSamplerType type;
	uint16_t set;
	uint16_t binding;
	uint16_t array_size;
} spoopy_shader_sampler_t;

typedef struct spoopy_shader_input {
	const char *name;
	uint16_t location;
	uint16_t num_locations_consumed;
} spoopy_shader_input_t;

struct spoopy_shader_reflection {
	spoopy_shader_block_t* uniform_buffers;
	spoopy_shader_sampler_t* samplers;
	spoopy_shader_input_t* inputs;
	uint16_t num_uniform_buffers;
	uint16_t num_samplers;
	uint16_t num_inputs;
};

#ifdef __cplusplus
}
#endif
