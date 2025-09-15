#pragma once

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
void video_set_mode(uint32_t display, uint32_t width, uint32_t height, bool resizeable);
void spoopy_video_shutdown(void);

#ifdef __cplusplus
}
#endif // extern "C"
