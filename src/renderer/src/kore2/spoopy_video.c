#include <spoopy.h>
#include <spoopy_video.h>
#include <spoopy_shader.h>
#include <spoopy_log.h>
#include <memory/spoopy_memory.h>
#include <kinc/system.h>

#include "../../../spoopy_system_info.h"
#include "spoopy_kinc_shader.h"

static bool video_initialized = false;

void spoopy_video_init(const spoopy_video_init_params_t* params) {
    if(SPOOPY_UNLIKELY(video_initialized)) {
        SPOOPY_LOG_WARN("Video subsystem already initialized.");
        return;
    }

    kinc_init(params->title, params->width, params->height, NULL, NULL);
    assert(spoopy_global_context_init());

    video_initialized = true;
}

void spoopy_video_shutdown(void) {
	spoopy_shader_cleanup();
    video_initialized = false;
}
