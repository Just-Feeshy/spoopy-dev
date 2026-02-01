#pragma once

#include <spoopy_shader.h>
#include <spoopy_vertex_attr.h>
#include <spoopy_color.h>
#include <spoopy_image.h>
#include <spoopy_types.h>
#include <spoopy_graphics.h>

typedef struct spoopy_backend_funcs {
	void (*init)(void);
	void (*shutdown)(void);
	void (*shader_init)(spoopy_shader_object_t* shader, spoopy_shader_source_t* info);
	spoopy_pipeline_t* (*pipeline_link)(uint32_t num_objs, spoopy_shader_object_t* objs[]);
	void (*pipeline_compile)(spoopy_pipeline_t* pipeline, uint32_t spec_count, spoopy_vertex_attr_spec_t spec[spec_count], uint32_t buffer_index);
	void (*shader_destroy)(spoopy_shader_object_t* shader, bool must_free);
	uint32_t (*pipeline_get_texture_unit)(spoopy_pipeline_t* pipeline, const char* name);
	void (*pipeline_bind)(spoopy_pipeline_t* pipeline);
	bool (*vertex_buffer_create)(spoopy_vertex_buffer_t* buffer, uint32_t capacity, uint32_t count, void* data, uint32_t stride);
	bool (*index_buffer_create)(spoopy_index_buffer_t* buffer, uint32_t count, void* data);
	void (*clear)(spoopy_graphics_t* graphics, spoopy_buffer_kind_t flags, const spoopy_color_t* color_val, float depth_val);
	void (*draw_mesh)(const spoopy_mesh_t* mesh, spoopy_pipeline_t* pipeline);
	void (*swap_buffers)(void);
	size_t (*texture_size)(void);
	void (*texture_create)(spoopy_texture_t* tex, const spoopy_texture_params_t* p);
	void (*texture_fill)(spoopy_texture_t* tex, uint32_t mipmap, uint32_t layer, const spoopy_image_t* img);
	void (*texture_set)(uint32_t unit, spoopy_texture_t* tex);
	void (*texture_destroy)(spoopy_texture_t* tex);
	size_t (*buffer_size)(spoopy_buffer_type_t type);
	spoopy_uniform_t* (*shader_uniform)(spoopy_pipeline_t* pipeline, const char* name);
	void (*uniform_set_int)(spoopy_uniform_t* uniform, int value);
	void (*uniform_set_int2)(spoopy_uniform_t* uniform, int v0, int v1);
	void (*uniform_set_int3)(spoopy_uniform_t* uniform, int v0, int v1, int v2);
	void (*uniform_set_int4)(spoopy_uniform_t* uniform, int v0, int v1, int v2, int v3);
	void (*uniform_set_ints)(spoopy_uniform_t* uniform, const int* values, int count);
	void (*uniform_set_float)(spoopy_uniform_t* uniform, float value);
	void (*uniform_set_float2)(spoopy_uniform_t* uniform, float v0, float v1);
	void (*uniform_set_float3)(spoopy_uniform_t* uniform, float v0, float v1, float v2);
	void (*uniform_set_float4)(spoopy_uniform_t* uniform, float v0, float v1, float v2, float v3);
	void (*uniform_set_floats)(spoopy_uniform_t* uniform, const float* values, int count);
	void (*uniform_set_bool)(spoopy_uniform_t* uniform, bool value);
	void (*uniform_set_matrix3)(spoopy_uniform_t* uniform, const float* values);
	void (*uniform_set_matrix4)(spoopy_uniform_t* uniform, const float* values);
	void* (*window_create_pointer)(uint32_t display);
} spoopy_backend_funcs_t;

extern spoopy_backend_funcs_t _backend_funcs;
