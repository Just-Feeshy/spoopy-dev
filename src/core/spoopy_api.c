#include <spoopy_api.h>
#include <spoopy_backend.h>
#include <spoopy_log.h>

void spoopy_api_shader_init(spoopy_shader_object_t* shader, spoopy_shader_source_t* info) {
	_backend_funcs.shader_init(shader, info);
}

spoopy_pipeline_t* spoopy_api_pipeline_link(uint32_t num_objs, spoopy_shader_object_t* objs[], uint32_t num_structs) {
	return _backend_funcs.spoopy_pipeline_link(num_objs, objs, num_structs);
}

void spoopy_api_pipeline_compile(spoopy_pipeline_t* pipeline, uint32_t spec_count, spoopy_vertex_attr_spec_t spec[spec_count], uint32_t structure) {
	_backend_funcs.pipeline_compile(pipeline, spec_count, spec, structure);
}
