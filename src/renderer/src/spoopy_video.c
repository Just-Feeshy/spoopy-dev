#include <spoopy.h>
#include <spoopy_video.h>
#include <spoopy_log.h>
#include <kore3/gpu/device.h>
#include <kore3/system.h>

#include "../../spoopy_system_info.h"

struct spoopy_device {
    kore_gpu_device_t* device;

    struct {
        spoopy_video_session_t* session;
        spoopy_video_compile_request_t* compile_request;
    } context;
};

void spoopy_video_init(const spoopy_video_init_params_t* params) {
    kore_init(params->title, params->width, params->height, NULL, NULL);
}

void spoopy_video_new_device(spoopy_device_t* device) {
    kore_gpu_device_wishlist wishlist = {0};
    kore_gpu_device_create(device->device, &wishlist);
    SLANG_CHECK(slang_createGlobalSession(SLANG_API_VERSION, &device->session));

    device->compile_request = spCreateCompileRequest(device->session);
    if(!device->compile_request) {
        spoopy_log_error("Failed to create compile request for the video device.");
        return;
    }
}

void spoopy_video_shutdown(void) {
    // Cleanup code here
}
