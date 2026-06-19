#pragma once

// Full credits go towards the Taisei Team for the pixmap file format handler design
// Very simple, straightforward, and efficient (just like their event system)
// Plus, very easy to expand upon for more personal use cases
// Also, it allows for easy addition of new file formats without modifying core code
// which is always a good thing

#include <spoopy.h>
#include <spoopy_types.h>

#ifdef __cplusplus
extern "C" {
#endif

// TODO (Framework): Something needs to be done about this.
typedef struct SDL_IOStream SDL_IOStream;

typedef struct spoopy_fileformat_handler {
	bool (*probe)(SDL_IOStream* stream);
	bool (*load)(SDL_IOStream* stream, spoopy_image_t* img);

	const char** file_extensions;
	const char* format_name;
} spoopy_fileformat_handler_t;

extern spoopy_fileformat_handler_t spoopy_fileformat_png;
// extern spoopy_fileformat_handler_t spoopy_fileformat_jpeg;
// extern spoopy_fileformat_handler_t spoopy_fileformat_svg;

#ifdef __cplusplus
}
#endif
