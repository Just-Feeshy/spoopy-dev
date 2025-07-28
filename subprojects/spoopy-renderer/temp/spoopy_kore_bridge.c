#include <spoopy_video.h>
#include <spoopy_shader.h>
#include <kore3/system.h>

// Bridge/glue code between Spoopy and Kore
// This file provides the abstraction layer that allows Spoopy to interface with Kore
// while maintaining clean separation of concerns and full cross-platform compatibility

// Off topic, Kore has caused me two days worth of pain and suffering..
// I bet you Kore is not even going to last two months before it gets replaced
// with SDL3 with custom middleware rendering

// Bridge function: Spoopy video initialization -> Kore initialization
void spoopy_video_init(const spoopy_video_init_params_t* params) {
    kore_init(params->title, params->width, params->height, NULL, NULL);
}

void spoopy_video_shutdown(void) {
    // Since Kore does not have a shutdown function, we can leave this empty
}

spoopy_shader_language_t spoopy_shader_required_language() {
#if defined(KORE_METAL)
    return SPOOPY_SHADER_LANGUAGE_MSL;
#elif defined(KORE_WEBGPU)
    return SPOOPY_SHADER_LANGUAGE_WGSL;
#else
    SPOOPY_LOG_ERROR("Unsupported shader language for Kore backend");
    return SPOOPY_SHADER_LANGUAGE_INVALID;
#endif
}

// Additional bridge functions can be added here for other Kore subsystems
// For example:
// - Audio bridging: spoopy_kore_audio_*()
// - Input bridging: spoopy_kore_input_*()
// - Graphics bridging: spoopy_kore_graphics_*()
// - Networking bridging: spoopy_kore_network_*()
