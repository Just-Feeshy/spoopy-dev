#include <spoopy.h>
#include "../spoopy_depth_image.h"

#include <format/spoopy_fileformats.h>
#include <memory/spoopy_memory.h>

#include <SDL3/SDL_error.h>
#include <SDL3/SDL_iostream.h>
#include <SDL3/SDL_surface.h>

static const uint8_t png_magic[] = { 0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A };

static bool spoopy_png_probe(SDL_IOStream* stream) {
	uint8_t magic[sizeof(png_magic)] = {0};
	SDL_ReadIO(stream, magic, sizeof(magic));
	return !memcmp(magic, png_magic, sizeof(png_magic));
}

static bool spoopy_png_decode(SDL_IOStream* stream, spoopy_image_t* img) {
	SDL_Surface* surf = SDL_LoadPNG_IO(stream, false);
	if(!surf) {
		SPOOPY_LOG_ERROR("SDL_LoadPNG_IO - ERROR: %s", SDL_GetError());
		return false;
	}

	bool ok = image_from_sdl_surface(surf, img);
	if(!ok) {
		SPOOPY_LOG_ERROR("Failed to convert the SDL surface to an image");
	}

	SDL_DestroySurface(surf);
	return ok;
}

spoopy_fileformat_handler_t spoopy_fileformat_png = {
	.format_name = "PNG",
	.file_extensions = (const char*[]) { "png", NULL },
	.probe = spoopy_png_probe,
	.load = spoopy_png_decode,
};
