#include <format/spoopy_fileformats.h>
#include <spoopy_image_mths.h>

#include <SDL3/SDL_iostream.h>
#include <SDL3/SDL_error.h>

static spoopy_fileformat_handler_t* fileformat_handlers[] = {
	[SPOOPY_IMAGE_FILE_FORMAT_PNG] = &spoopy_fileformat_png,
};

static spoopy_fileformat_handler_t* spoopy_image_probe_stream(SDL_IOStream* stream) {
	const size_t handler_count = sizeof(fileformat_handlers)
		/ sizeof(fileformat_handlers[0]);

	for(size_t i=0; i<handler_count; ++i) {
		spoopy_fileformat_handler_t* h = fileformat_handlers[i];
		if(!h->probe) {
			continue;
		}

		bool match = h->probe(stream);
		if(SDL_SeekIO(stream, 0, SDL_IO_SEEK_SET) < 0) {
			SPOOPY_LOG_ERROR("Failed to seek stream back to start after probing: %s", SDL_GetError());
			return NULL;
		}

		if(match) {
			return h;
		}
	}

	return NULL;
}

static spoopy_fileformat_handler_t* spoopy_handler_for_format(spoopy_image_file_format_t file_format) {
	uint32_t index = (uint32_t)file_format;
	assert(index < SPOOPY_IMAGE_NUM_FILE_FORMATS);
	return fileformat_handlers[index];
}

bool spoopy_image_load_stream(SDL_IOStream* stream, spoopy_image_file_format_t file_format, spoopy_image_t* dst) {
	spoopy_fileformat_handler_t* handler = NULL;

	if(file_format == SPOOPY_IMAGE_FILE_FORMAT_AUTO) {
		handler = spoopy_image_probe_stream(stream);
	}else {
		handler = spoopy_handler_for_format(file_format);
	}

	if(SPOOPY_UNLIKELY(handler == NULL)) {
		SPOOPY_LOG_ERROR("No suitable image file format handler found");
		return false;
	}

	if(SPOOPY_UNLIKELY(!handler->load)) {
		SPOOPY_LOG_ERROR("Failed to load image using format handler: %s", handler->format_name);
		return false;
	}

    return handler->load(stream, dst);
}

bool spoopy_image_load_callbacks(spoopy_file_read_callbacks_t* callbacks, void* user_data, spoopy_image_file_format_t file_format, spoopy_image_t* dst) {
    SPOOPY_UNUSED(callbacks);
    SPOOPY_UNUSED(user_data);
    SPOOPY_UNUSED(file_format);
    SPOOPY_UNUSED(dst);
    return false;
}

// TODO (Older Systems): Use uint instead
uint32_t spoopy_image_data_size(spoopy_pixel_format_t format, uint32_t width, uint32_t height) {
	assert(width >= 1);
	assert(height >= 1);
	uint64_t pixel_size = spoopy_pixel_format_size(format);
	assert(pixel_size >= 1);
	uint64_t s = (uint64_t)width * (uint64_t)height * pixel_size;
	assert(s <= INT32_MAX);
	return s;
}

void* spoopy_image_alloc_buffer(spoopy_pixel_format_t format, uint32_t width, uint32_t height, uint32_t* out_bufsize) {
	uint32_t s = spoopy_image_data_size(format, width, height);

	if(out_bufsize) {
		*out_bufsize = s;
	}

	return spoopy_heap_alloc(s);
}

void* spoopy_image_alloc_buffer_for_copy(spoopy_image_t* src, uint32_t* out_bufsize) {
	return spoopy_image_alloc_buffer(src->format, src->width, src->height, out_bufsize);
}
