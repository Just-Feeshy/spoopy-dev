#include <spoopy_backend.h>
#include <spoopy_pipeline.h>
#include "kore2.h"

const size_t spoopy_shader_object_size = sizeof(spoopy_shader_object_t);

#ifdef KORE_METAL
spoopy_shader_object_t* spoopy_mtl_shader_init(spoopy_shader_object_t* shader, spoopy_shader_source_t* info);
#else
spoopy_shader_object_t* spoopy_kinc_shader_init(spoopy_shader_object_t* shader, spoopy_shader_source_t* info);
#endif

spoopy_pipeline_t* spoopy_kinc_pipeline_link(uint32_t num_objs, spoopy_shader_object_t* objs[], uint32_t num_structs);
void spoopy_kinc_pipeline_compile(spoopy_pipeline_t* pipeline, spoopy_shader_object_t* vertex_shader, uint32_t spec_count, spoopy_vertex_attr_spec_t spec[spec_count], uint32_t structure);

spoopy_backend_funcs_t _backend_funcs = {
#ifdef KORE_METAL
    .shader_init = spoopy_mtl_shader_init,
#else
	.shader_init = spoopy_kinc_shader_init,
#endif

	.spoopy_pipeline_link = spoopy_kinc_pipeline_link,
	.pipeline_compile = spoopy_kinc_pipeline_compile
};
