#ifdef SPOOPY_VIDEO_H
#define SPOOPY_VIDEO_H

#include <spoopy.h>

typedef struct spoopy_video_init_params {
    const char* title;
    int width;
    int height;
} spoopy_video_init_params_t;

void spoopy_video_init(const spoopy_video_init_params_t params);
void spoopy_video_shutdown(void);

#endif // SPOOPY_VIDEO_H
