#ifndef SPOOPY_API_H
#define SPOOPY_API_H

// For more general API functions for the user to use.
// Everything here is meant to be used by the developer

#define SPOOPY_MAIN_PROJECT
#include <spoopy.h>

#ifdef __cplusplus
extern "C" {
#endif

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

#undef SPOOPY_MAIN_PROJECT

#endif
