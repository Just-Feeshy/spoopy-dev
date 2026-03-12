#pragma once

#include <spoopy.h>
#include <spoopy_image.h>
#include <spoopy_types.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct SDL_IOStream SDL_IOStream;

SPOOPY_FUNC_CORE bool spoopy_image_load_stream(SDL_IOStream* stream, spoopy_image_file_format_t file_format, spoopy_image_t* dst);
SPOOPY_FUNC_CORE bool spoopy_image_load_callbacks(spoopy_file_read_callbacks_t* callbacks, void* user_data, spoopy_image_file_format_t file_format, spoopy_image_t* dst);

#ifdef __cplusplus
}
#endif
