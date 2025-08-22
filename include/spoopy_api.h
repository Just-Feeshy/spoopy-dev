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

#ifdef __cplusplus
extern "C" {
#endif

spoopy_pipeline_t* spoopy_api_pipeline_link(uint32_t num_objs, spoopy_shader_object_t* objs[], uint32_t num_structs);
void spoopy_api_pipeline_compile(spoopy_pipeline_t* pipeline, uint32_t spec_count, spoopy_vertex_attr_spec_t spec[spec_count], uint32_t structure);
void spoopy_api_shader_init(spoopy_shader_object_t* shader, spoopy_shader_source_t* info);
void spoopy_api_add_macro(spoopy_transpile_options_t* options, const char* name, const char* value);
bool spoopy_api_shader_supported(spoopy_transpile_options_t* transpile_opts, const spoopy_shader_lang_t* info);
bool spoopy_api_should_quit(void);

bool spoopy_api_shader_transpile(
	spoopy_shader_source_t* source,
	spoopy_shader_source_t* target,
	spoopy_transpile_options_t* transpile_opts
);

#ifdef __cplusplus
}
#endif
