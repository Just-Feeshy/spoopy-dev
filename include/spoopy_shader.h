#pragma once

#include <spoopy_graphics.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct spoopy_shader_lang spoopy_shader_lang_t;
typedef struct spoopy_shader_object spoopy_shader_object_t;
typedef struct spoopy_context spoopy_context_t;

typedef enum spoopy_optimization_level {
    SPOOPY_OPTIMIZATION_LEVEL_NONE = 0,
    SPOOPY_OPTIMIZATION_LEVEL_DEFAULT,
    SPOOPY_OPTIMIZATION_LEVEL_HIGH,
    SPOOPY_OPTIMIZATION_LEVEL_MAXIMAL,
} spoopy_optimization_level_t;

typedef enum spoopy_shader_stage {
	SPOOPY_STAGE_INVALID = 0,
	SPOOPY_STAGE_VERTEX = 1,
	SPOOPY_STAGE_FRAGMENT = 5,
} spoopy_shader_stage_t;

typedef struct spoopy_shader_macro {
    const char* name;
    const char* value;
} spoopy_shader_macro_t;

typedef struct spoopy_transpile_options {
    spoopy_shader_macro_t* macros;
	size_t macro_count;
    // spoopy_optimization_level_t optimization_level;
    spoopy_shader_stage_t stage;
    const char* filename;
	uint8_t flags;
} spoopy_transpile_options_t;

typedef struct spoopy_shader_source {
    const char* content;
	size_t content_size;
	spoopy_shader_stage_t stage;
	spoopy_renderer_t target;
    const char* entry_point;
    const char* module_name; // Optional, can be NULL
} spoopy_shader_source_t;

extern spoopy_context_t global_context;
extern const size_t spoopy_shader_object_size;

bool spoopy_global_context_init(void);
void spoopy_shader_cleanup(void);

#ifdef __cplusplus
}
#endif // extern "C"
