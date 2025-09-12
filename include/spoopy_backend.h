#pragma once

#include <spoopy_shader.h>
#include <spoopy_pipeline.h>
#include <spoopy_vertex_attr.h>
#include <spoopy_buffers.h>

typedef struct spoopy_backend_funcs {
	spoopy_shader_object_t* (*shader_init)(spoopy_shader_object_t* shader, spoopy_shader_source_t* info);
	void (*shader_destroy)(spoopy_shader_object_t* shader);
	spoopy_pipeline_t* (*spoopy_pipeline_link)(uint32_t num_objs, spoopy_shader_object_t* objs[], uint32_t num_structs);
	void (*pipeline_compile)(spoopy_pipeline_t* pipeline, spoopy_shader_object_t* vertex_shader, uint32_t spec_count, spoopy_vertex_attr_spec_t spec[spec_count], uint32_t structure);
	spoopy_vertex_buffer_t* (*vertex_buffer_create)(uint32_t capacity, uint32_t count, void* data, uint32_t structure, spoopy_pipeline_t* pipeline);
	spoopy_index_buffer_t* (*index_buffer_create)(uint32_t count, void* data);
} spoopy_backend_funcs_t;

extern spoopy_backend_funcs_t _backend_funcs;
