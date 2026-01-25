#pragma once

#include <spoopy.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct spoopy_graphics spoopy_graphics_t;

void spoopy_graphics_init(void);
spoopy_graphics_t* spoopy_graphics_new(void);
bool spoopy_graphics_set_mode(spoopy_graphics_t* graphics, void* context_view);
void spoopy_graphics_set_gamma_correction(bool gamma_correction);
bool spoopy_graphics_get_gamma_correction(void);

#ifdef __cplusplus
}
#endif
