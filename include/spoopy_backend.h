#pragma once

#include <spoopy_shader.h>

typedef struct spoopy_backend_funcs {
	void (*shader_init)(spoopy_shader_object_t* shader, spoopy_shader_source_t* info);
} spoopy_backend_funcs_t;

extern spoopy_backend_funcs_t _backend_funcs;

#undef SPOOPY_IMPL
