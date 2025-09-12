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

void __video_internal_bind(void);
void __video_internal_shutdown(void);

void spoopy_video_init(const spoopy_video_init_params_t* params);
void spoopy_video_shutdown(void);

#ifdef __cplusplus
}
#endif // extern "C"
