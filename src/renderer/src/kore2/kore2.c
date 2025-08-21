#include <spoopy_backend.h>

void spoopy_kinc_shader_init(spoopy_shader_object_t* shader, spoopy_shader_source_t* info);

spoopy_backend_funcs_t _backend_funcs = {
    .shader_init = spoopy_kinc_shader_init,
};
