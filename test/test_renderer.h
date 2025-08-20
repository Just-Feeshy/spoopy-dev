#pragma once // For more modern C, use #pragma once instead of include guards

#include <spoopy_api.h>

static void test_init(void) {
    SPOOPY_LOG_INFO("Test Renderer Initialized");

#ifndef __EMSCRIPTEN__
    spoopy_sdl_thread_init();
#endif

    spoopy_video_init(&(spoopy_video_init_params_t) {
        .title = "Test Renderer",
        .width = 800,
        .height = 600
    });
}

static spoopy_shader_object_t* load_shader(const char* src, spoopy_shader_stage_t stage) {
    spoopy_shader_source_t source = {
        .context = src,
        .context_size = strlen(src),
        .stage = stage,
        .entry_point = "main",
        .module_name = "shader",
        .lang = {
            .target = SLANG_METAL_LIB,
            .profile = "metallib_2_0"
        }
    };

    spoopy_transpile_options_t transpile_opts = {
        .filename = "<embedded>"
    };

    if(!spoopy_api_shader_supported(&transpile_opts, &source.lang)) {
        SPOOPY_LOG_ERROR("Shader target not supported: %d", source.lang.target);
        return NULL;
    }

    spoopy_shader_source_t new_src;
    bool result = spoopy_api_shader_transpile(
        &source,
        &new_src,
        &transpile_opts
    );

    if(!result) {
        SPOOPY_LOG_ERROR("Failed to transpile shader: %s", source.entry_point);
        return NULL;
    }

	SPOOPY_LOG_SUCCESS("Shader transpiled successfully: %s", source.entry_point);
	spoopy_shader_object_t* shader = spoopy_heap_alloc(spoopy_shader_object_size);
	spoopy_api_shader_init(shader, &new_src);

    SPOOPY_LOG_SUCCESS("Shader compiled and ready for use: %s", source.entry_point);
    return shader;
}
