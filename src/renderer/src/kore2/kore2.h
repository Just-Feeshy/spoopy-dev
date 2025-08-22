#pragma once

#include <kinc/graphics5/pipeline.h>
#include <kinc/graphics5/shader.h>

#ifdef __cplusplus
extern "C" {
#endif

struct spoopy_pipeline {
    kinc_g5_pipeline_t core;
	kinc_g5_vertex_structure_t structures[];
};

struct spoopy_shader_object {
	kinc_g5_shader_t core;
};

#ifdef __cplusplus
}
#endif
