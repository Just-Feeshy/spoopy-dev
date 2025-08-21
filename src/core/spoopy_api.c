#include <spoopy_api.h>
#include <spoopy_backend.h>
#include <spoopy_log.h>

void spoopy_api_shader_init(spoopy_shader_object_t* shader, spoopy_shader_source_t* info) {
	SPOOPY_LOG_INFO("API shader_init - target: %d, entry_point: %s", info->lang.target, info->entry_point);
	_backend_funcs.shader_init(shader, info);
}
