#include <spoopy.h>
#include <format/spoopy_fileformats.h>
#include <spoopy_image.h>
#include <memory/spoopy_memory.h>

#include <SDL3/SDL_error.h>
#include <SDL3/SDL_iostream.h>
#include <png.h>
#include <setjmp.h>

static const uint8_t png_magic[] = { 0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A };

SPOOPY_ATTR_UNUSED static void spoopy_png_rwops_write_data(png_structp png_ptr, png_bytep data, png_size_t length) {
	SDL_IOStream* out = png_get_io_ptr(png_ptr);
	SDL_WriteIO(out, data, length);
}

SPOOPY_ATTR_UNUSED static void spoopy_png_rwops_read_data(png_structp png_ptr, png_bytep data, png_size_t length) {
	SDL_IOStream* out = png_get_io_ptr(png_ptr);
	SDL_ReadIO(out, data, length);
}

SPOOPY_ATTR_UNUSED static void spoopy_png_init_rwops_read(png_structp png, SDL_IOStream* rwops) {
	png_set_read_fn(png, rwops, spoopy_png_rwops_read_data);
}

SPOOPY_ATTR_UNUSED static void spoopy_png_init_rwops_write(png_structp png, SDL_IOStream* rwops) {
	png_set_write_fn(png, rwops, spoopy_png_rwops_write_data, NULL);
}

SPOOPY_ATTR_UNUSED static void spoopy_png_error_handler(png_structp png_ptr, png_const_charp msg) {
	SPOOPY_LOG_ERROR("PNG error: %s", msg);
	longjmp(png_jmpbuf(png_ptr), 1);
}

SPOOPY_ATTR_UNUSED static void spoopy_png_warning_handler(png_structp png_ptr, png_const_charp msg) {
	SPOOPY_UNUSED(png_ptr);
	SPOOPY_LOG_WARN("PNG warning: %s", msg);
}

SPOOPY_ATTR_UNUSED static void spoopy_png_setup_error_handling(png_structp png_ptr) {
	png_set_error_fn(png_ptr, NULL, spoopy_png_error_handler, spoopy_png_warning_handler);
}

static PNG_CALLBACK(png_voidp, spoopy_png_malloc, (png_structp png, png_alloc_size_t size)) {
	SPOOPY_UNUSED(png);
	return spoopy_heap_alloc((size_t)size);
}

static PNG_CALLBACK(void, spoopy_png_free, (png_structp png, png_voidp ptr)) {
	SPOOPY_UNUSED(png);
	spoopy_heap_free(ptr);
}

SPOOPY_ATTR_UNUSED static png_structp spoopy_png_create_read_struct(void) {
	png_structp png = png_create_read_struct_2(
		PNG_LIBPNG_VER_STRING,
		NULL,
		spoopy_png_error_handler,
		spoopy_png_warning_handler,
		NULL,
		spoopy_png_malloc,
		spoopy_png_free
	);
	return png;
}

SPOOPY_ATTR_UNUSED static png_structp spoopy_png_create_write_struct(void) {
	png_structp png = png_create_write_struct_2(
		PNG_LIBPNG_VER_STRING,
		NULL,
		spoopy_png_error_handler,
		spoopy_png_warning_handler,
		NULL,
		spoopy_png_malloc,
		spoopy_png_free
	);
	return png;
}

static bool spoopy_png_probe(SDL_IOStream* stream) {
	uint8_t magic[sizeof(png_magic)] = {0};
	SDL_ReadIO(stream, magic, sizeof(magic));
	return !memcmp(magic, png_magic, sizeof(png_magic));
}

static spoopy_image_layout_t clrtype_to_layout(int color_type) {
	switch(color_type) {
		case PNG_COLOR_TYPE_RGB:
			return SPOOPY_IMAGE_LAYOUT_RGB;
		case PNG_COLOR_TYPE_RGB_ALPHA:
			return SPOOPY_IMAGE_LAYOUT_RGBA;
		case PNG_COLOR_TYPE_GRAY:
		case PNG_COLOR_TYPE_GRAY_ALPHA:
			return SPOOPY_IMAGE_LAYOUT_R;
		case PNG_COLOR_TYPE_PALETTE:
			return SPOOPY_IMAGE_LAYOUT_RGB;
		default:
			return SPOOPY_IMAGE_LAYOUT_INVALID;
	}
}

static bool spoopy_png_decode(SDL_IOStream* stream, spoopy_image_t* img) {
	png_structp png = NULL;
	png_infop info_ptr = NULL;

	int bit_depth = 0;
	int color_type = 0;
	const char *volatile error = NULL;

	img->pixels.raw_data = NULL;

	if(!(png = spoopy_png_create_read_struct())) {
		error = "Failed to create PNG read struct";
		goto finally;
	}

	if(!(info_ptr = png_create_info_struct(png))) {
		error = "Failed to create PNG info struct";
		goto finally;
	}

	if(setjmp(png_jmpbuf(png))) {
		error = "Error during PNG read";
		goto finally;
	}

	spoopy_png_init_rwops_read(png, stream);
	png_read_info(png, info_ptr);
	png_get_IHDR(png, info_ptr, NULL, NULL, &bit_depth, &color_type, NULL, NULL, NULL);
	png_set_alpha_mode(png, PNG_ALPHA_PNG, PNG_DEFAULT_sRGB);

	/* Read any color_type into 8bit depth, RGBA format. */
	png_set_expand(png);

	png_set_gray_to_rgb(png);

	if(bit_depth == 16) {
		png_set_expand_16(png);
	}

#if SDL_BYTEORDER == SDL_LIL_ENDIAN
	png_set_swap(png);
#endif

	int num_passes = png_set_interlace_handling(png);
	png_read_update_info(png, info_ptr);

	int channels = png_get_channels(png, info_ptr);
	color_type = png_get_color_type(png, info_ptr);
	bit_depth = png_get_bit_depth(png, info_ptr);

	assert(
		(color_type == PNG_COLOR_TYPE_RGB && channels == 3) ||
		(color_type == PNG_COLOR_TYPE_RGB_ALPHA && channels == 4)
	);
	assert(bit_depth == 8 || bit_depth == 16);

	img->width = png_get_image_width(png, info_ptr);
	img->height = png_get_image_height(png, info_ptr);
	img->format = SPOOPY_IMAGE_MAKE_FORMAT(
		clrtype_to_layout(color_type),
		(uint8_t)bit_depth
	);

	img->origin = SPOOPY_IMAGE_ORIGIN_TOP_LEFT;

	const png_size_t rowbytes = png_get_rowbytes(png, info_ptr);
	const uint32_t h = img->height;

	if(rowbytes == 0 || h == 0) {
		error = "Invalid PNG image dimensions";
		goto finally;
	}

	if(rowbytes > SIZE_MAX / h) {
		error = "PNG image size is too large";
		goto finally;
	}

	png_bytep buffer = img->pixels.raw_data = spoopy_heap_alloc((size_t)(rowbytes * h));

	for(int pass = 0; pass < num_passes; pass++) {
		for(uint32_t y = 0; y < img->height; y++) {
			png_read_row(png, buffer + y * rowbytes, NULL);
		}
	}

	png_read_end(png, info_ptr);

finally:
	if(png != NULL) {
		png_destroy_read_struct(&png, NULL, NULL);
	}

	if(error != NULL) {
		SPOOPY_LOG_ERROR("%s", error);

		if(img->pixels.raw_data != NULL) {
			spoopy_heap_free(img->pixels.raw_data);
			img->pixels.raw_data = NULL;
		}

		img->format = SPOOPY_IMAGE_FORMAT_INVALID;
		return false;
	}

	return true;
}

spoopy_fileformat_handler_t spoopy_fileformat_png = {
	.format_name = "PNG",
	.file_extensions = (const char*[]) { "png", NULL },
	.probe = spoopy_png_probe,
	.load = spoopy_png_decode,
};
