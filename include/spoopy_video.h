#ifndef SPOOPY_VIDEO_H
#define SPOOPY_VIDEO_H

#ifdef __cplusplus
extern "C" {
#endif

typedef SlangSession spoopy_video_session_t;

typedef struct spoopy_video_init_params {
    const char* title;
    int width;
    int height;
} spoopy_video_init_params_t;

typedef struct spoopy_device spoopy_device_t;

void spoopy_video_init(const spoopy_video_init_params_t* params);
void spoopy_video_new_device(spoopy_device_t* device);
void spoopy_video_shutdown(void);

#ifdef __cplusplus
}
#endif

#endif // SPOOPY_VIDEO_H
