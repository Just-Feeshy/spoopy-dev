#include <audio/spoopy_audio.h>

#include "spoopy_audio_backend.h"

void spoopy_audio_init(const spoopy_audio_request_t request, uint16_t num_players, int channels_per_player[num_players]) {
	_sp_aud_backend_init(request, num_players, channels_per_player);
}

bool spoopy_audio_play(spoopy_audio_stream_t* stream, uint16_t player_index, int chan, bool loop, double position) {
	return _sp_aud_backend.funcs.play(stream, player_index, chan, loop, position);
}

uint8_t spoopy_audio_get_bits_per_sample(void) {
	return _sp_aud_backend.funcs.get_bits_per_sample();
}

uint16_t spoopy_audio_get_raw_format(void) {
	return _sp_aud_backend.funcs.get_raw_format();
}

void spoopy_audio_shutdown(void) {
	_sp_aud_backend.funcs.shutdown();
}
