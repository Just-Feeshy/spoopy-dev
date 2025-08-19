#include <spoopy_api.h>
#include <spoopy_backend.h>

void spoopy_api_shader_init(spoopy_shader_object_t* shader, spoopy_shader_source_t* info) {
	_backend_funcs.shader_init(shader, info);
}
