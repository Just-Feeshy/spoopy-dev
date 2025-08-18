#pragma once // For more modern C, use #pragma once instead of include guards

#include <spoopy.h>
#include <spoopy_log.h>
#include <spoopy_thread.h>
#include <spoopy_video.h>

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
