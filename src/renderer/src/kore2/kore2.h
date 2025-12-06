#pragma once

#include <kinc/graphics4/pipeline.h>
#include <kinc/backend/graphics4/pipeline.h>
#include <kinc/graphics4/vertexbuffer.h>
#include <kinc/graphics4/indexbuffer.h>
#include <kinc/graphics4/vertexstructure.h>
#include <kinc/graphics4/shader.h>
#include <kinc/graphics4/texture.h>
#include <kinc/graphics5/sampler.h>
#include <kinc/graphics4/graphics.h>
#include <kinc/graphics5/constantlocation.h>
#include <kinc/math/matrix.h>
#include <kinc/image.h>
#include <kinc/system.h>
#include <kinc/window.h>
#include <kinc/display.h>
#include <spoopy_types.h>

#ifdef __cplusplus
extern "C" {
#endif

struct spoopy_pipeline {
    kinc_g4_pipeline_t core;
	kinc_g4_vertex_structure_t structures[];
};

struct spoopy_shader_object {
	kinc_g4_shader_t core;
	uint32_t attr_count;
	const char* attr_names[];
};

// TODO (Optimize Memory) - This is a temporary struct until we have a proper one
struct spoopy_vertex_buffer {
	kinc_g4_vertex_buffer_t raw;
};

struct spoopy_index_buffer {
	kinc_g4_index_buffer_t raw;
};

struct spoopy_texture {
	kinc_g4_texture_t raw;
	kinc_g5_sampler_t sampler;
	bool sampler_initialized;
	spoopy_texture_params_t params;
	bool has_params;
};

struct spoopy_uniform {
	struct spoopy_pipeline* pipeline;
	kinc_g4_constant_location_t location;
};

#ifdef __cplusplus
}
#endif
