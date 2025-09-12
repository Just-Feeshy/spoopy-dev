#pragma once

#include <kinc/graphics4/pipeline.h>
#include <kinc/graphics4/vertexbuffer.h>
#include <kinc/graphics4/indexbuffer.h>
#include <kinc/graphics4/vertexstructure.h>
#include <kinc/graphics4/shader.h>

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

// TODO (Optimize Memory) - This is a temporary struct until we have a proper
struct spoopy_vertex_buffer {
	kinc_g4_vertex_buffer_t raw;
};

struct spoopy_index_buffer {
	kinc_g4_index_buffer_t raw;
};

#ifdef __cplusplus
}
#endif
