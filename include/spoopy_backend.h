#ifndef SPOOPY_BACKEND_H
#define SPOOPY_BACKEND_H

#include <spoopy_shader.h>

typedef struct pipeline_renderer_funcs {
	void (*compile)(SPOOPY_IMPL(device)* device, spoopy_shader_source_t* info);
} pipeline_renderer_funcs_t;

extern pipeline_renderer_funcs_t _pipeline_funcs;

#undef SPOOPY_IMPL
#endif
