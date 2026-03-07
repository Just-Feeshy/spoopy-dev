#pragma once

#include <spoopy_types.h>

#ifdef __cplusplus
extern "C" {
#endif

void __spoopy_video_internal_init_backend(void);

void spoopy_video_init(const spoopy_video_init_params_t* params);
void spoopy_video_update_mode(uint32_t display, uint32_t width, uint32_t height);
void spoopy_video_set_mode(uint32_t display, uint32_t width, uint32_t height);
void spoopy_video_shutdown(void);
void spoopy_video_get_viewport_size(float* width, float* height);

#ifdef __cplusplus
}
#endif // extern "C"
