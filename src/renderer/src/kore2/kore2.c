#ifndef SPOOPY_KORE2_C
#define SPOOPY_KORE2_C

#include <spoopy_backend.h>
#include <kinc/graphics5/shader.h>

#include "spoopy_kinc_shader.h"

size_t spoopy_shader_object_size = sizeof(spoopy_shader_object_t);

spoopy_backend_funcs_t _backend_funcs = {
    .shader_init = spoopy_kinc_shader_init,
};

#endif
