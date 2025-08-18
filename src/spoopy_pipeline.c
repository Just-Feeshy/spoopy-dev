#include <spoopy_api.h>

void spoopy_api_pipeline_compile(spoopy_device_t* device, spoopy_shader_source_t* info) {
	_pipeline_funcs.compile(device->device, info);
}
