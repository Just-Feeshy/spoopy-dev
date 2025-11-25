#include <spoopy.h>
#include <spoopy_image.h>

#include "kore2.h"

static size_t read_callback(void *user_data, void *data, size_t size) {
	return kinc_file_reader_read((kinc_file_reader_t *)user_data, data, size);
}

static size_t size_callback(void *user_data) {
	return kinc_file_reader_size((kinc_file_reader_t *)user_data);
}

static size_t pos_callback(void *user_data) {
	return kinc_file_reader_pos((kinc_file_reader_t *)user_data);
}

static void seek_callback(void *user_data, size_t pos) {
	kinc_file_reader_seek((kinc_file_reader_t *)user_data, pos);
}

bool spoopy_kinc_image_read_callback(spoopy_file_read_callbacks_t* img_callback, const char* path) {
	kinc_file_reader_t reader;
	if(kinc_file_reader_open(&reader, path, KINC_FILE_TYPE_ASSET)) {
		img_callback.read = read_callback;
		img_callback.size = size_callback;
		img_callback.pos = pos_callback;
		img_callback.seek = seek_callback;

		return true;
	}

	return false;
}
