#include <spoopy.h>
#include <spoopy_video.h>
#include <spoopy_context.h>
#include <spoopy_log.h>
#include <kore3/gpu/device.h>
#include <kore3/system.h>

#include "../../spoopy_system_info.h"

struct spoopy_device {
    kore_gpu_device* device;
    spoopy_context_t* context;
};

void spoopy_video_init(const spoopy_video_init_params_t* params) {
    kore_init(params->title, params->width, params->height, NULL, NULL);
}

void spoopy_video_new_device(spoopy_device_t* device) {
    kore_gpu_device_wishlist wishlist = {0};
    kore_gpu_device_create(device->device, &wishlist);
    assert(spoopy_new_context(device->context));
}

void spoopy_video_shutdown(void) {
    // Cleanup code here
}
