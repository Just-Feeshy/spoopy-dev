#ifndef SPOOPY_DEFS_H
#define SPOOPY_DEFS_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum spoopy_shader_stage {
    SPOOPY_SHADER_STAGE_VERTEX,
    SPOOPY_SHADER_STAGE_FRAGMENT,
    SPOOPY_SHADER_STAGE_INVALID
} spoopy_shader_stage_t;

typedef enum spoopy_shader_language {
    SPOOPY_SHADER_LANGUAGE_INVALID
    SPOOPY_SHADER_LANGUAGE_GLSL,
    SPOOPY_SHADER_LANGUAGE_HLSL,
    SPOOPY_SHADER_LANGUAGE_SPIRV,
    SPOOPY_SHADER_LANGUAGE_MSL,
    SPOOPY_SHADER_LANGUAGE_WGSL, // Not supported yet
} spoopy_shader_language_t;

typedef struct spoopy_shader_info {
    spoopy_shader_stage_t stage;
    spoopy_shader_language_t language;
} spoopy_shader_info_t;

typedef struct spoopy_shader_source {
    const char* source;
    const char* entry_point;
    size_t content_length;
    spoopy_shader_info_t info;
} spoopy_shader_source_t;

#ifdef __cplusplus
}
#endif

#endif // SPOOPY_DEFS_H
