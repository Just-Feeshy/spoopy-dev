#pragma once // For more modern C, use #pragma once instead of include guards
#include <spoopy.h>
#include <spoopy_log.h>
#include <spoopy_thread.h>

static void test_init(void) {
    SPOOPY_LOG_INFO("Test Renderer Initialized");

    spoopy_sdl_thread_init();
}
