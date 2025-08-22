#include <spoopy_shader.h>
#include <utils/assert.h>
#include <spoopy_log.h>

#include "kore2.h"

const size_t spoopy_shader_object_size = sizeof(spoopy_shader_object_t);

void spoopy_kinc_shader_init(spoopy_shader_object_t* shader, spoopy_shader_source_t* info) {
	assert(shader != NULL && info != NULL);

	switch(info->stage) {
		case SPOOPY_STAGE_VERTEX:
			SPOOPY_LOG_WARN("Content: %.*s", (int)info->content_size, (const char*)info->content);
			kinc_g5_shader_init(&shader->core, info->content, info->content_size, KINC_G5_SHADER_TYPE_VERTEX);
			break;
		case SPOOPY_STAGE_FRAGMENT:
			kinc_g5_shader_init(&shader->core, info->content, info->content_size, KINC_G5_SHADER_TYPE_FRAGMENT);
			break;
		default:
			SPOOPY_LOG_WARN("Must have a valid shader stage to initialize the shader object. Received: [SPOOPY_STAGE_INVALID]");
			return;
	}
}
