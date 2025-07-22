#ifdef SPOOPY_KORE_VIDEO_H
#define SPOOPY_KORE_VIDEO_H

#include <spoopy_system.h>
#include <kore3/system.h>

void spoopy_video_init(const spoopy_video_init_params_t params) {
    kore_init(params.title, params.width, params.height);
}

#endif
