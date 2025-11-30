#include <spoopy_backend.h>
#include <spoopy_pipeline.h>
#include <spoopy_types.h>
#include <spoopy_log.h>
#include <spoopy_image.h>

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
uint32_t spoopy_kinc_pipeline_get_texture_unit(spoopy_pipeline_t* pipeline, const char* name);
void spoopy_kinc_pipeline_bind(spoopy_pipeline_t* pipeline);
void spoopy_kinc_pipeline_compile(spoopy_pipeline_t* pipeline, spoopy_shader_object_t* vertex_shader, uint32_t spec_count, spoopy_vertex_attr_spec_t spec[spec_count], uint32_t structure);
void spoopy_kinc_pipeline_bind(spoopy_pipeline_t* pipeline);
void spoopy_kinc_texture_create(spoopy_texture_t* tex, const spoopy_texture_params_t* p);
void spoopy_kinc_texture_fill(spoopy_texture_t* tex, uint32_t mipmap, uint32_t layer, const spoopy_image_t* img);
void spoopy_kinc_texture_set(uint32_t unit, spoopy_texture_t* tex);
void spoopy_kinc_texture_destroy(spoopy_texture_t* tex);
spoopy_uniform_t spoopy_kinc_shader_uniform(spoopy_pipeline_t* pipeline, const char* name);
void spoopy_kinc_uniform_set_int(spoopy_uniform_t uniform, int value);
void spoopy_kinc_uniform_set_int2(spoopy_uniform_t uniform, int v0, int v1);
void spoopy_kinc_uniform_set_int3(spoopy_uniform_t uniform, int v0, int v1, int v2);
void spoopy_kinc_uniform_set_int4(spoopy_uniform_t uniform, int v0, int v1, int v2, int v3);
void spoopy_kinc_uniform_set_ints(spoopy_uniform_t uniform, const int* values, int count);
void spoopy_kinc_uniform_set_float(spoopy_uniform_t uniform, float value);
void spoopy_kinc_uniform_set_float2(spoopy_uniform_t uniform, float v0, float v1);
void spoopy_kinc_uniform_set_float3(spoopy_uniform_t uniform, float v0, float v1, float v2);
void spoopy_kinc_uniform_set_float4(spoopy_uniform_t uniform, float v0, float v1, float v2, float v3);
void spoopy_kinc_uniform_set_floats(spoopy_uniform_t uniform, const float* values, int count);
void spoopy_kinc_uniform_set_bool(spoopy_uniform_t uniform, bool value);
void spoopy_kinc_uniform_set_matrix3(spoopy_uniform_t uniform, const float* values);
void spoopy_kinc_uniform_set_matrix4(spoopy_uniform_t uniform, const float* values);

static void spoopy_kinc_shader_destroy(spoopy_shader_object_t* shader) {
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

static void spoopy_kinc_begin_frame(void) {
	kinc_g4_begin(0);
}

static void spoopy_kinc_clear(spoopy_buffer_kind_t flags, const spoopy_color_t* color_val, float depth_val) {
	kinc_g4_clear(flags, color_val ? color_val->packed : 0, depth_val, 0);
}

static void spoopy_kinc_draw_mesh(const spoopy_mesh_t* mesh, spoopy_pipeline_t* pipeline) {
	if (pipeline == NULL) {
		SPOOPY_LOG_ERROR("Cannot draw mesh with NULL pipeline");
		return;
	}
	kinc_g4_set_vertex_buffer(&mesh->vertex_buffer->raw);
	kinc_g4_set_index_buffer(&mesh->index_buffer->raw);

	kinc_g4_draw_indexed_vertices();
}

static void spoopy_kinc_swap_buffers(void) {
	kinc_g4_end(0);
	kinc_g4_swap_buffers();
}

const size_t spoopy_kinc_texture_size(void) {
	return sizeof(spoopy_texture_t);
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
	.pipeline_get_texture_unit = spoopy_kinc_pipeline_get_texture_unit,
	.pipeline_compile = spoopy_kinc_pipeline_compile,
	.pipeline_bind = spoopy_kinc_pipeline_bind,
	.begin_frame = spoopy_kinc_begin_frame,
	.clear = spoopy_kinc_clear,
	.draw_mesh = spoopy_kinc_draw_mesh,
	.swap_buffers = spoopy_kinc_swap_buffers,
	.texture_size = spoopy_kinc_texture_size,
	.texture_create = spoopy_kinc_texture_create,
	.texture_fill = spoopy_kinc_texture_fill,
	.texture_set = spoopy_kinc_texture_set,
	.texture_destroy = spoopy_kinc_texture_destroy,
	.shader_uniform = spoopy_kinc_shader_uniform,
	.uniform_set_int = spoopy_kinc_uniform_set_int,
	.uniform_set_int2 = spoopy_kinc_uniform_set_int2,
	.uniform_set_int3 = spoopy_kinc_uniform_set_int3,
	.uniform_set_int4 = spoopy_kinc_uniform_set_int4,
	.uniform_set_ints = spoopy_kinc_uniform_set_ints,
	.uniform_set_float = spoopy_kinc_uniform_set_float,
	.uniform_set_float2 = spoopy_kinc_uniform_set_float2,
	.uniform_set_float3 = spoopy_kinc_uniform_set_float3,
	.uniform_set_float4 = spoopy_kinc_uniform_set_float4,
	.uniform_set_floats = spoopy_kinc_uniform_set_floats,
	.uniform_set_bool = spoopy_kinc_uniform_set_bool,
	.uniform_set_matrix3 = spoopy_kinc_uniform_set_matrix3,
	.uniform_set_matrix4 = spoopy_kinc_uniform_set_matrix4,
};
