#pragma once

#include <spoopy.h>
#include <spoopy_image.h>
#include <spoopy_types.h>

#ifdef __cplusplus
extern "C" {
#endif

// TODO (Framework): Something needs to be done about this.
typedef struct SDL_IOStream SDL_IOStream;

SPOOPY_FUNC_CORE bool spoopy_image_load_stream(SDL_IOStream* stream, spoopy_image_file_format_t file_format, spoopy_image_t* dst);
SPOOPY_FUNC_CORE bool spoopy_image_load_callbacks(spoopy_file_read_callbacks_t* callbacks, void* user_data, spoopy_image_file_format_t file_format, spoopy_image_t* dst);
SPOOPY_FUNC_CORE uint32_t spoopy_image_data_size(spoopy_pixel_format_t format, uint32_t width, uint32_t height);
SPOOPY_FUNC_CORE void* spoopy_image_alloc_buffer(spoopy_pixel_format_t format, uint32_t width, uint32_t height, uint32_t* out_bufsize);
SPOOPY_FUNC_CORE void* spoopy_image_alloc_buffer_for_copy(spoopy_image_t* src, uint32_t* out_bufsize);

#ifdef __cplusplus
}
#endif
