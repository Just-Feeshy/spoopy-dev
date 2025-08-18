#ifndef SPOOPY_PIPELINE_H
#define SPOOPY_PIPELINE_H

#include <spoopy_shader.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct pipeline_renderer_funcs {
	void (*compile)(spoopy_shader_lang_t* info);
} pipeline_renderer_funcs_t;

void spoopy_gpu_pipeline_compile(spoopy_shader_lang_t* info);

#ifdef __cplusplus
}
#endif

#endif
