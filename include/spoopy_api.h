#pragma once

// For more general API functions for the user to use.
// Everything here is meant to be used by the developer

#include <spoopy.h>
#include <memory/spoopy_memory.h>
#include <spoopy_shader.h>
#include <spoopy_thread.h>
#include <spoopy_log.h>
#include <spoopy_video.h>
#include <spoopy_app.h>

#ifdef __cplusplus
extern "C" {
#endif

void spoopy_api_shader_init(spoopy_shader_object_t* shader, spoopy_shader_source_t* info);
void spoopy_api_add_macro(spoopy_transpile_options_t* options, const char* name, const char* value);
bool spoopy_api_shader_supported(spoopy_transpile_options_t* transpile_opts, const spoopy_shader_lang_t* info);
bool spoopy_api_shader_transpile(
	spoopy_shader_source_t* source,
	spoopy_shader_source_t* target,
	spoopy_transpile_options_t* transpile_opts
);

#ifdef __cplusplus
}
#endif // extern "C"
