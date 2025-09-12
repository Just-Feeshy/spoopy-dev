#include <spoopy.h>
#include <spoopy_video.h>
#include <spoopy_shader.h>
#include <spoopy_log.h>
#include <memory/spoopy_memory.h>
#include <internal/spoopy_internal_window.h>

#include <kinc/system.h>

#ifdef __APPLE__
#include <objc/objc.h>
typedef struct objc_object NSWindow;
NSWindow *kinc_get_mac_window_handle(int window_index);
#endif

#include "../../../spoopy_system_info.h"

bool video_initialized = false;

static bool handle_kinc_window_close(void *data) {
    (void)data;
    spoopy_api_request_quit();
    return false; // Prevent immediate window close, let spoopy handle it gracefully
}

void spoopy_video_init(const spoopy_video_init_params_t* params) {
    if(SPOOPY_UNLIKELY(video_initialized)) {
        SPOOPY_LOG_WARN("Video subsystem already initialized.");
        return;
    }

	spoopy_sdl_window_init();
    kinc_init(params->title, params->width, params->height, NULL, NULL);
    kinc_window_set_close_callback(0, handle_kinc_window_close, NULL);

    assert(spoopy_global_context_init());

#ifdef __APPLE__
	spoopy_sdl_window_create(kinc_get_mac_window_handle(0));
#endif

	// Setup desired slang profile and family based on the current backend
	spoopy_slang_family = 0;
	desired_slang_pf = NULL;

#if defined(KORE_METAL)
    desired_slang_pf = "metallib_2_3";

    spoopy_slang_family  |= (1 << SLANG_METAL);
#elif defined(KORE_DIRECT3D11)
    desired_slang_pf = "sm_4_0";

    spoopy_slang_family  |= (1 << SLANG_DXBC)
                         |  (1 << SLANG_DXBC_ASM)
                         |  (1 << SLANG_DXIL)
                         |  (1 << SLANG_DXIL_ASM)
                         |  (1 << SLANG_HLSL);
#elif defined(KORE_DIRECT3D12)
    desired_slang_pf = "sm_4_0";

    spoopy_slang_family  |= (1 << SLANG_DXIL)
                         |  (1 << SLANG_DXIL_ASM)
                         |  (1 << SLANG_HLSL);
#elif defined(KORE_VULKAN)
    desired_slang_pf = "spirv_1_0";

    spoopy_slang_family  |= (1 << SLANG_SPIRV)
                         |  (1 << SLANG_SPIRV_ASM);
#endif

    video_initialized = true;
}

void spoopy_video_shutdown(void) {
	spoopy_shader_cleanup();
    video_initialized = false;
}
