#include "kore2.h"

#include <spoopy_log.h>

#include <kinc/math/matrix.h>

#include <string.h>

static bool kinc_constant_location_valid(kinc_g4_constant_location_t location) {
	const ConstantLocation5Impl* impl = &location.impl._location5.impl;
	return impl->vertexOffset >= 0 || impl->fragmentOffset >= 0 || impl->computeOffset >= 0;
}

spoopy_uniform_t spoopy_kinc_shader_uniform(spoopy_pipeline_t* pipeline, const char* name) {
	spoopy_uniform_t uni = {
		.pipeline = pipeline,
		.type = SPOOPY_UNIFORM_TYPE_UNKNOWN,
		.location = {0},
	};
	kinc_g4_constant_location_t location = kinc_g4_pipeline_get_constant_location(&pipeline->core, name);

	if(!kinc_constant_location_valid(location)) {
		SPOOPY_LOG_ERROR("Uniform '%s' not found in pipeline", name);
		goto final;
	}

	uni.location = location;

final: // Nothing yet, but I will be needing it later
	return uni;
}

void spoopy_kinc_uniform_set_int(spoopy_uniform_t uniform, int value) {
	kinc_g4_set_int(uniform.location, value);
}

void spoopy_kinc_uniform_set_int2(spoopy_uniform_t uniform, int v0, int v1) {
	kinc_g4_set_int2(uniform.location, v0, v1);
}

void spoopy_kinc_uniform_set_int3(spoopy_uniform_t uniform, int v0, int v1, int v2) {
	kinc_g4_set_int3(uniform.location, v0, v1, v2);
}

void spoopy_kinc_uniform_set_int4(spoopy_uniform_t uniform, int v0, int v1, int v2, int v3) {
	kinc_g4_set_int4(uniform.location, v0, v1, v2, v3);
}

void spoopy_kinc_uniform_set_ints(spoopy_uniform_t uniform, const int* values, int count) {
	if(values == NULL || count <= 0) {
		SPOOPY_LOG_ERROR("Attempted to set integer uniform array with invalid data");
		return;
	}

	kinc_g4_set_ints(uniform.location, (int*)values, count);
}

void spoopy_kinc_uniform_set_float(spoopy_uniform_t uniform, float value) {
	kinc_g4_set_float(uniform.location, value);
}

void spoopy_kinc_uniform_set_float2(spoopy_uniform_t uniform, float v0, float v1) {
	kinc_g4_set_float2(uniform.location, v0, v1);
}

void spoopy_kinc_uniform_set_float3(spoopy_uniform_t uniform, float v0, float v1, float v2) {
	kinc_g4_set_float3(uniform.location, v0, v1, v2);
}

void spoopy_kinc_uniform_set_float4(spoopy_uniform_t uniform, float v0, float v1, float v2, float v3) {
	kinc_g4_set_float4(uniform.location, v0, v1, v2, v3);
}

void spoopy_kinc_uniform_set_floats(spoopy_uniform_t uniform, const float* values, int count) {
	if(values == NULL || count <= 0) {
		SPOOPY_LOG_ERROR("Attempted to set float uniform array with invalid data");
		return;
	}

	kinc_g4_set_floats(uniform.location, (float*)values, count);
}

void spoopy_kinc_uniform_set_bool(spoopy_uniform_t uniform, bool value) {
	kinc_g4_set_bool(uniform.location, value);
}

static bool spoopy_uniform_copy_matrix(float* dst, size_t element_count, const float* values) {
	if(values == NULL) {
		SPOOPY_LOG_ERROR("Attempted to set matrix uniform with NULL data");
		return false;
	}

	memcpy(dst, values, element_count * sizeof(float));
	return true;
}

void spoopy_kinc_uniform_set_matrix3(spoopy_uniform_t uniform, const float* values) {
	kinc_matrix3x3_t mat;
	if(!spoopy_uniform_copy_matrix(mat.m, 3 * 3, values)) {
		return;
	}

	kinc_g4_set_matrix3(uniform.location, &mat);
}

void spoopy_kinc_uniform_set_matrix4(spoopy_uniform_t uniform, const float* values) {
	kinc_matrix4x4_t mat;
	if(!spoopy_uniform_copy_matrix(mat.m, 4 * 4, values)) {
		return;
	}

	kinc_g4_set_matrix4(uniform.location, &mat);
}
