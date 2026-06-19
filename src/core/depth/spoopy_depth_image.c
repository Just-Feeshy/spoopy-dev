#include "spoopy_depth_image.h"

#include <spoopy_image_mths.h>

// Stripped from Taisei
// https://github.com/taisei-project/taisei/blob/master/src/pixmap/pixmap.c

static SDL_PixelFormat sdl_pixfmt_conversion(SDL_PixelFormat fmt) {
	if(SDL_ISPIXELFORMAT_FLOAT(fmt)) {
		if(SDL_BYTESPERPIXEL(fmt) > 8) {
			return SDL_PIXELFORMAT_RGBA128_FLOAT;
		}else {
			return SDL_PIXELFORMAT_RGBA64_FLOAT;
		}
	}

	if(SDL_ISPIXELFORMAT_10BIT(fmt) || SDL_BYTESPERPIXEL(fmt) > 4) {
		return SDL_PIXELFORMAT_RGBA64;
	}

	return SDL_PIXELFORMAT_RGBA32;
}

static spoopy_pixel_format_t sdl_pixfmt_to_spoopy(SDL_PixelFormat fmt) {
	switch(fmt) {
		case SDL_PIXELFORMAT_RGB48:         return SPOOPY_PIXEL_FORMAT_RGB16;
		case SDL_PIXELFORMAT_RGB48_FLOAT:   return SPOOPY_PIXEL_FORMAT_RGB16F;
		case SDL_PIXELFORMAT_RGB96_FLOAT:   return SPOOPY_PIXEL_FORMAT_RGB32F;
		case SDL_PIXELFORMAT_RGBA32:        return SPOOPY_PIXEL_FORMAT_RGBA8;
		case SDL_PIXELFORMAT_RGBA64:        return SPOOPY_PIXEL_FORMAT_RGBA16;
		case SDL_PIXELFORMAT_RGBA64_FLOAT:  return SPOOPY_PIXEL_FORMAT_RGBA16;
		case SDL_PIXELFORMAT_RGBA128_FLOAT: return SPOOPY_PIXEL_FORMAT_RGBA32;
		default: return 0;
	}
}

bool image_from_sdl_surface(SDL_Surface* surf, spoopy_image_t* img) {
	assert(!img->pixels.raw_data);

	SDL_PixelFormat target_sdl_fmt = sdl_pixfmt_conversion(surf->format);
	SDL_Surface *temp_surf = NULL;
	bool ok = false;

	if(target_sdl_fmt != surf->format) {
		temp_surf = SDL_ConvertSurface(surf, target_sdl_fmt);

		if(!temp_surf) {
			SPOOPY_LOG_ERROR("SDL_ConvertSurface - ERROR: %s", SDL_GetError());
			return false;
		}

		surf = temp_surf;
	}

	spoopy_pixel_format_t px_fmt = sdl_pixfmt_to_spoopy(surf->format);
	if(!px_fmt) {
		SPOOPY_LOG_ERROR("Unable to %s map to a supported format", SDL_GetPixelFormatName(surf->format));
		goto fail;
	}

	img->width  = surf->w;
	img->height = surf->h;
	img->format = px_fmt;
	img->pixels.raw_data = spoopy_image_alloc_buffer_for_copy(img, &img->data_size);

	assert(surf->pitch == surf->w * spoopy_pixel_format_size(img->format));
	assert(!SDL_MUSTLOCK(surf));
	memcpy(img->pixels.raw_data, surf->pixels, img->data_size);
	ok = true;

fail:
	if(temp_surf) {
		SDL_DestroySurface(temp_surf);
	}

	return ok;
}
