#pragma once

#include <spoopy.h>

#include <audio/spoopy_audio.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct spoopy_audio_stream spoopy_audio_stream_t;

typedef struct _audio_funcs {
	bool (*init)(const spoopy_audio_request_t want, uint16_t num_players, int channels_per_player[num_players]);
	bool (*play)(spoopy_audio_stream_t* stream, uint16_t player_index, int chan, bool loop, double position);
	uint8_t (*get_bits_per_sample)(void);
	uint16_t (*get_raw_format)(void);
	bool (*shutdown)(void);
} _audio_funcs_t;

typedef struct spoopy_audio_backend {
	const char* name;
	_audio_funcs_t funcs;
} spoopy_audio_backend_t;

extern spoopy_audio_backend_t _sp_aud_backend;

bool _sp_aud_backend_init(const spoopy_audio_request_t want, uint16_t num_players, int channels_per_player[num_players]);

#ifdef __cplusplus
}
#endif
