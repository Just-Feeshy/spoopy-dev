#include <spoopy_video.h>

// Forward declarations for bridge functions
void spoopy_kore_video_init(const spoopy_video_init_params_t* params);
void spoopy_kore_video_shutdown(void);

void spoopy_video_init(const spoopy_video_init_params_t* params) {
    // Use the bridge function instead of calling Kore directly
    spoopy_kore_video_init(params);
}

void spoopy_video_shutdown(void) {
    // Use the bridge function instead of calling Kore directly
    spoopy_kore_video_shutdown();
}
