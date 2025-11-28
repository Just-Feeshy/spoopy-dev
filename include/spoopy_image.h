#pragma once

#include <spoopy.h>
#include <spoopy_color.h>

#define SPOOPY_IMAGE_MAKE_FORMAT(layout, depth) (((uint32_t)(layout) << 8) | (uint32_t)(depth))

#define SPOOPY_IMAGE_FORMAT_LAYOUT(fmt) (((fmt) >> 8) & 0xFF)
#define SPOOPY_IMAGE_FORMAT_DEPTH(fmt)  ((fmt) & 0xFF)


#ifdef __cplusplus
extern "C" {
#endif

typedef enum spoopy_image_file_format {
	SPOOPY_IMAGE_FILE_FORMAT_AUTO = -1,

	SPOOPY_IMAGE_FILE_FORMAT_PNG,

	SPOOPY_IMAGE_NUM_FILE_FORMATS
} spoopy_image_file_format_t;

 typedef enum spoopy_image_layout {
	SPOOPY_IMAGE_LAYOUT_INVALID = -1,
	SPOOPY_IMAGE_LAYOUT_R = 1,
	SPOOPY_IMAGE_LAYOUT_RG,
	SPOOPY_IMAGE_LAYOUT_RGB,
	SPOOPY_IMAGE_LAYOUT_RGBA,
	SPOOPY_IMAGE_LAYOUT_BGRA,
} spoopy_image_layout_t;

typedef enum spoopy_image_format {
	SPOOPY_IMAGE_FORMAT_INVALID = -1,

	SPOOPY_IMAGE_FORMAT_GREY8 = SPOOPY_IMAGE_MAKE_FORMAT(SPOOPY_IMAGE_LAYOUT_R, 8),

	SPOOPY_IMAGE_FORMAT_A16 = SPOOPY_IMAGE_MAKE_FORMAT(SPOOPY_IMAGE_LAYOUT_R, 16),

	SPOOPY_IMAGE_FORMAT_RGB24 = SPOOPY_IMAGE_MAKE_FORMAT(SPOOPY_IMAGE_LAYOUT_RGB, 24),

	SPOOPY_IMAGE_FORMAT_RGBA32 = SPOOPY_IMAGE_MAKE_FORMAT(SPOOPY_IMAGE_LAYOUT_RGBA, 32),
	SPOOPY_IMAGE_FORMAT_A32 = SPOOPY_IMAGE_MAKE_FORMAT(SPOOPY_IMAGE_LAYOUT_R, 32),
	SPOOPY_IMAGE_FORMAT_BGRA32 = SPOOPY_IMAGE_MAKE_FORMAT(SPOOPY_IMAGE_LAYOUT_BGRA, 32),

	SPOOPY_IMAGE_FORMAT_RGBA64 = SPOOPY_IMAGE_MAKE_FORMAT(SPOOPY_IMAGE_LAYOUT_RGBA, 64),

	SPOOPY_IMAGE_FORMAT_RGBA128 = SPOOPY_IMAGE_MAKE_FORMAT(SPOOPY_IMAGE_LAYOUT_RGBA, 128),

	SPOOPY_IMAGE_FORMAT_FIRST = SPOOPY_IMAGE_FORMAT_GREY8,
	SPOOPY_IMAGE_FORMAT_LAST = SPOOPY_IMAGE_FORMAT_RGBA128,
	SPOOPY_IMAGE_FORMAT_COUNT = SPOOPY_IMAGE_FORMAT_LAST - SPOOPY_IMAGE_FORMAT_FIRST + 1
} spoopy_image_format_t;

typedef enum spoopy_image_origin {
	SPOOPY_IMAGE_ORIGIN_TOP_LEFT,
	SPOOPY_IMAGE_ORIGIN_BOTTOM_LEFT,
} spoopy_image_origin_t;

typedef struct spoopy_image_save_options {
	spoopy_image_file_format_t file_format;
} spoopy_image_save_options_t;

typedef struct spoopy_image {
	union {
		void* raw_data;

		spoopy_color1_t* color1;
		spoopy_color2_t* color2;
		spoopy_color3_t* color3;
		spoopy_color_t* color;
	} pixels;

	uint32_t width;
	uint32_t height;
	uint32_t data_size;
	spoopy_image_format_t format;
	spoopy_image_origin_t origin;
} spoopy_image_t;

#ifdef __cplusplus
}
#endif
