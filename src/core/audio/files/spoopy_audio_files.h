#pragma once

#include <spoopy.h>
#include <spoopy_types.h>
#include <audio/spoopy_stream.h>
#include <SDL3/SDL_iostream.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _audio_file_func {
	bool (*probe)(const uint8_t* data, int32_t length);
	bool (*open)(SDL_IOStream* rw, spoopy_audio_stream_t* stream, uint8_t buf_size, uint8_t buf[buf_size]);
} _audio_file_func_t;

typedef struct spoopy_audio_file {
	const char* name;
	_audio_file_func_t funcs;
} spoopy_audio_file_t;

bool spoopy_audio_file_open(spoopy_audio_stream_t* stream, SDL_IOStream* rw, spoopy_audio_file_format_t file_format);

#ifdef __cplusplus
}
#endif
