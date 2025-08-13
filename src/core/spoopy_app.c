// #include <spoopy_rhi.h>
#include <spoopy_app.h>

void spoopy_app_init(void) { // "Flixelize" this IG
    spoopy_sdl_thread_init();
    // spoopy_new_rhi();
}

void spoopy_app_shutdown(void) {
    // TODO (States): Add shutdown for states, "Flixelize" this IG
    spoopy_sdl_thread_shutdown();
