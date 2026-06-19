#pragma once

#include <spoopy.h>
#include <audio/spoopy_player.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct spoopy_mixer {
	spoopy_stream_spec_t spec;
	uint16_t players_size;

	// TODO (Channel): Have SFX streams

	spoopy_stream_player_t players[];
} spoopy_mixer_t;

SPOOPY_FUNC_CORE bool spoopy_mixer_init(spoopy_mixer_t* mixer, const spoopy_stream_spec_t* spec, uint16_t players_size, int num_channels_arr[players_size]) SPOOPY_ATTR_NONNULL(1);
SPOOPY_FUNC_CORE spoopy_audio_stream_t spoopy_mixer_load(const char* path, spoopy_audio_file_format_t file_format);
SPOOPY_FUNC_CORE void spoopy_mixer_shutdown(spoopy_mixer_t* mixer);

SPOOPY_FUNC_CORE void spoopy_mixer_process(spoopy_mixer_t* mx, size_t buf_size, void* buffer);

#ifdef __cplusplus
}
#endif
