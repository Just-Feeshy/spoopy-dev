#ifndef SPOOPY_VIDEO_H
#define SPOOPY_VIDEO_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct spoopy_device spoopy_device_t;

typedef struct spoopy_video_init_params {
    const char* title;
    int width;
    int height;
} spoopy_video_init_params_t;

void spoopy_video_init(const spoopy_video_init_params_t* params);
void spoopy_video_shutdown(void);

#ifdef __cplusplus
}
#endif // extern "C"

#endif
