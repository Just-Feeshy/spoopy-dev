#include "../spoopy_audio_backend.h"

static bool aud_null_init(const spoopy_audio_request_t want, uint16_t num_players, int channels_per_player[num_players]) {
	(void)want;
	(void)channels_per_player;
	return false;
}

static bool aud_null_play(spoopy_audio_stream_t* stream, uint16_t player_index, int chan,  bool loop, double position) {
	(void)stream;
	(void)player_index;
	(void)chan;
	(void)loop;
	(void)position;
	return false;
}

static uint8_t aud_null_get_bits_per_sample(void) { return 16; }
static bool aud_null_shutdown(void) { return false; }

spoopy_audio_backend_t _sp_backend_null = {
	.name = "sdl",
	.funcs = {
		.init = aud_null_init,
		.play = aud_null_play,
		.get_bits_per_sample = aud_null_get_bits_per_sample,
		.shutdown = aud_null_shutdown,
	},
};
