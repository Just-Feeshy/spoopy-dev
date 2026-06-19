#pragma once

#include <spoopy_image.h>
#include <SDL3/SDL_surface.h>

#ifdef __cplusplus
extern "C" {
#endif

bool image_from_sdl_surface(SDL_Surface* surface, spoopy_image_t* img);

#ifdef __cplusplus
}
#endif
