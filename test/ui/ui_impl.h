#pragma once

#include <spoopy.h>

typedef union SDL_Event SDL_Event;

#ifdef __cplusplus
extern "C" {
#endif

bool spoopy_ui_impl_init(void);
void spoopy_ui_impl_process_event(const SDL_Event* event);
void spoopy_ui_impl_new_frame(void);
void spoopy_ui_impl_render(void);
void spoopy_ui_impl_shutdown(void);

#ifdef __cplusplus
}
#endif
