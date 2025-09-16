#pragma once

// Don't care about having SD: header here
// Not meant to be included in the public API

void spoopy_sdl_window_init(void);
void spoopy_sdl_window_create(void* raw_handle, uint32_t width, uint32_t height, bool resizeable);
bool spoopy_init_main_window(void);
