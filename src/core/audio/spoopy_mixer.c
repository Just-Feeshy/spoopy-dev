#include <audio/spoopy_mixer.h>

#include <SDL3/SDL_iostream.h>

#include "files/spoopy_audio_files.h"

bool spoopy_mixer_init(spoopy_mixer_t* mixer, const spoopy_stream_spec_t* spec, uint16_t players_size, int num_channels_arr[players_size]) {
	memset(mixer, 0, sizeof(*mixer) + players_size * sizeof(spoopy_stream_player_t));

	for(size_t i=0; i<players_size; ++i) {
		if(!spoopy_player_init(mixer->players + i, num_channels_arr[i], spec)) {
			SPOOPY_LOG_ERROR("spoopy_player_init() Failed!");
			return false;
		}
	}

	// TODO (Channel): Have SFX streams

	mixer->players_size = players_size;
	mixer->spec = *spec;
	return true;
}

// TODO(Framework): Have some kind of like vfs (This goes for 'rw')
spoopy_audio_stream_t spoopy_mixer_load(const char* path, spoopy_audio_file_format_t file_format) {
	SDL_IOStream* rw = SDL_IOFromFile(path, "rb");

	if(!rw) {
		SPOOPY_LOG_ERROR("Something happened while trying to read path");
		return (spoopy_audio_stream_t) { 0 };
	}

	spoopy_audio_stream_t stream;
	if(!spoopy_audio_file_open(&stream, rw, file_format)) {
		SDL_CloseIO(rw);
		return (spoopy_audio_stream_t) { 0 };
	}

#ifdef SPOOPY_BUILD_DEBUG
	SPOOPY_LOG_SUCCESS("Successfully loaded stream from %s", path);
#endif
	return stream;
}

void spoopy_mixer_shutdown(spoopy_mixer_t* mixer) {
	for(uint16_t i=0; i<mixer->players_size; ++i) {
		spoopy_stream_player_t* plr = mixer->players + i;

		if(plr->channels.data) {
			spoopy_player_shutdown(plr);
			memset(plr, 0, sizeof(spoopy_stream_player_t));
		}
	}
}

void spoopy_mixer_process(spoopy_mixer_t *mx, size_t buf_size, void *buffer) {
	for(int i=0; i<mx->players_size; ++i) {
		spoopy_stream_player_t* plr = mx->players + i;
		spoopy_player_process(plr, buf_size, buffer);
	}
}
