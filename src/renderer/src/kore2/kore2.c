#include <spoopy_backend.h>
#include <spoopy_pipeline.h>
#include <spoopy_types.h>
#include <spoopy_log.h>
#include "kore2.h"

const size_t spoopy_shader_object_size = sizeof(spoopy_shader_object_t);

#ifdef KORE_METAL
spoopy_shader_object_t* spoopy_mtl_shader_init(spoopy_shader_object_t* shader, spoopy_shader_source_t* info);
#else
spoopy_shader_object_t* spoopy_kinc_shader_init(spoopy_shader_object_t* shader, spoopy_shader_source_t* info);
#endif

spoopy_vertex_buffer_t* spoopy_kinc_vertex_buffer_create(uint32_t capacity, uint32_t count, void* data, uint32_t structure, spoopy_pipeline_t* pipeline);
spoopy_index_buffer_t* spoopy_kinc_index_buffer_create(uint32_t count, void* data);
spoopy_pipeline_t* spoopy_kinc_pipeline_link(uint32_t num_objs, spoopy_shader_object_t* objs[], uint32_t num_structs);
void spoopy_kinc_pipeline_compile(spoopy_pipeline_t* pipeline, spoopy_shader_object_t* vertex_shader, uint32_t spec_count, spoopy_vertex_attr_spec_t spec[spec_count], uint32_t structure);

void spoopy_kinc_shader_destroy(spoopy_shader_object_t* shader) {
	if (shader == NULL) {
		SPOOPY_LOG_WARN("Attempted to destroy a NULL shader object.");
		return;
	}

	kinc_g4_shader_destroy(&shader->core);
	for (uint32_t i = 0; i < shader->attr_count; i++) {
		if (shader->attr_names[i] != NULL) {
			spoopy_heap_free((void*)shader->attr_names[i]);
		}
	}

	spoopy_heap_free(shader);
}

void spoopy_kinc_begin_frame(void) {
	kinc_g4_begin(0);
}

void spoopy_kinc_clear(spoopy_buffer_kind_t flags, const spoopy_color_t* color_val, float depth_val) {
	kinc_g4_clear(flags, color_val ? color_val->packed : 0, depth_val, 0);
}

void spoopy_kinc_draw_mesh(const spoopy_mesh_t* mesh, spoopy_pipeline_t* pipeline) {
	kinc_g4_set_pipeline(&pipeline->core);
	kinc_g4_set_vertex_buffer(&mesh->vertex_buffer->raw);
	kinc_g4_set_index_buffer(&mesh->index_buffer->raw);
	kinc_g4_draw_indexed_vertices();
}

void spoopy_kinc_swap_buffers(void) {
	kinc_g4_end(0);
	kinc_g4_swap_buffers();
}

spoopy_backend_funcs_t _backend_funcs = {
#ifdef KORE_METAL
    .shader_init = spoopy_mtl_shader_init,
#else
	.shader_init = spoopy_kinc_shader_init,
#endif

	.vertex_buffer_create = spoopy_kinc_vertex_buffer_create,
	.index_buffer_create = spoopy_kinc_index_buffer_create,
	.shader_destroy = spoopy_kinc_shader_destroy,
	.spoopy_pipeline_link = spoopy_kinc_pipeline_link,
	.pipeline_compile = spoopy_kinc_pipeline_compile,
	.begin_frame = spoopy_kinc_begin_frame,
	.clear = spoopy_kinc_clear,
	.draw_mesh = spoopy_kinc_draw_mesh,
	.swap_buffers = spoopy_kinc_swap_buffers
};
