#include <spoopy.h>
#include <spoopy_video.h>
#include <spoopy_shader.h>
#include <spoopy_log.h>
#include <memory/spoopy_memory.h>
#include <kinc/system.h>

#include "../../../spoopy_system_info.h"

static bool video_initialized = false;

void spoopy_video_init(const spoopy_video_init_params_t* params) {
    if(SPOOPY_UNLIKELY(video_initialized)) {
        SPOOPY_LOG_WARN("Video subsystem already initialized.");
        return;
    }

    kinc_init(params->title, params->width, params->height, NULL, NULL);
    assert(spoopy_global_context_init());

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
