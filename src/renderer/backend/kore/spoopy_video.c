#include <spoopy_video.h>
#include <kore3/system.h>

void spoopy_video_init(const spoopy_video_init_params_t* params) {
    kore_init(params->title, params->width, params->height, NULL, NULL);
}

void spoopy_video_shutdown(void) {
    // Not yet
}
