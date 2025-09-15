#pragma once

// For more general API functions for the user to use.
// Everything here is meant to be used by the developer

#include <spoopy.h>
#include <memory/spoopy_memory.h>
#include <spoopy_shader.h>
#include <spoopy_thread.h>
#include <spoopy_log.h>
#include <spoopy_video.h>
#include <spoopy_pipeline.h>
#include <spoopy_vertex_attr.h>
#include <spoopy_events.h>
#include <spoopy_color.h>
#include <spoopy_types.h>

#ifdef __cplusplus
extern "C" {
#endif

SPOOPY_FUNC_CORE spoopy_pipeline_t* spoopy_api_pipeline_link(uint32_t num_objs, spoopy_shader_object_t* objs[], uint32_t num_structs);
SPOOPY_FUNC_CORE void spoopy_api_pipeline_compile(spoopy_pipeline_t* pipeline, spoopy_shader_object_t* vertex_shader, uint32_t spec_count, spoopy_vertex_attr_spec_t spec[spec_count], uint32_t structure);
SPOOPY_FUNC_CORE spoopy_shader_object_t* spoopy_api_shader_init(spoopy_shader_object_t* shader, spoopy_shader_source_t* info);
SPOOPY_FUNC_CORE spoopy_vertex_buffer_t* spoopy_api_vertex_buffer_create(uint32_t capacity, uint32_t count, void* data, uint32_t structure, spoopy_pipeline_t* pipeline);
SPOOPY_FUNC_CORE spoopy_index_buffer_t* spoopy_api_index_buffer_create(uint32_t count, void* data);
SPOOPY_FUNC_CORE void spoopy_api_shader_destroy(spoopy_shader_object_t* shader);
SPOOPY_FUNC_CORE void spoopy_api_add_macro(spoopy_transpile_options_t* options, const char* name, const char* value);
SPOOPY_FUNC_CORE bool spoopy_api_shader_supported(spoopy_transpile_options_t* transpile_opts, const spoopy_shader_lang_t* info);
SPOOPY_FUNC_CORE void spoopy_api_begin_frame(void);
SPOOPY_FUNC_CORE void spoopy_api_clear(spoopy_buffer_kind_t flags, const spoopy_color_t* color_val, float depth_val);
SPOOPY_FUNC_CORE void spoopy_api_draw_mesh(const spoopy_mesh_t* mesh, spoopy_pipeline_t* pipeline);
SPOOPY_FUNC_CORE void spoopy_api_swap_buffers(void);

SPOOPY_FUNC_CORE bool spoopy_api_should_quit(void);
SPOOPY_FUNC_CORE void spoopy_api_request_quit(void);

SPOOPY_FUNC_CORE bool spoopy_api_shader_transpile(
	spoopy_shader_source_t* source,
	spoopy_shader_source_t* target,
	spoopy_transpile_options_t* transpile_opts
);

#ifdef __cplusplus
}
#endif
