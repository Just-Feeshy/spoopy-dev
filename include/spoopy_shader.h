#ifndef SPOOPY_SPOOPY_SHADER_H
#define SPOOPY_SPOOPY_SHADER_H

#include <spoopy.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef int spoopy_shader_target_t;

typedef enum spoopy_optimization_level {
    SPOOPY_OPTIMIZATION_LEVEL_NONE = 0,
    SPOOPY_OPTIMIZATION_LEVEL_DEFAULT,
    SPOOPY_OPTIMIZATION_LEVEL_HIGH,
    SPOOPY_OPTIMIZATION_LEVEL_MAXIMAL,
} spoopy_optimization_level_t;

typedef enum spoopy_shader_stage {
	SPOOPY_STAGE_INVALID,
	SPOOPY_STAGE_VERTEX,
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
    spoopy_shader_target_t target;
    const char* profile;
    const char* filename;
	uint8_t flags;
} spoopy_transpile_options_t;

typedef struct spoopy_shader_source {
    const char* context;
	size_t context_size;
    spoopy_shader_stage_t stage;
    const char* entry_point;
    const char* module_name; // Optional, can be NULL
} spoopy_shader_source_t;

// I prefer to use char* instead of void* for clarity
typedef struct spoopy_shader_info {
	spoopy_shader_target_t target; // The target platform for the shader
	const char* profile;
} spoopy_shader_info_t;

typedef struct spoopy_context spoopy_context_t;
extern spoopy_context_t global_context_pool;

bool spoopy_global_context_init(void);
void spoopy_shader_cleanup(void);

#ifdef __cplusplus
}
#endif // extern "C"

#endif
