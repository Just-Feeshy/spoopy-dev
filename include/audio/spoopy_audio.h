#pragma once

#include <spoopy.h>
#include <spoopy_types.h>
#include <audio/spoopy_stream.h>

#ifdef __cplusplus
extern "C" {
#endif

SPOOPY_FUNC_CORE void spoopy_audio_init(const spoopy_audio_request_t request, uint16_t num_players, int channels_per_player[num_players]);
SPOOPY_FUNC_CORE bool spoopy_audio_play(spoopy_audio_stream_t* stream, uint16_t player_index, int chan, bool loop, double position);
SPOOPY_FUNC_CORE uint8_t spoopy_audio_get_bits_per_sample(void);
SPOOPY_FUNC_CORE uint16_t spoopy_audio_get_raw_format(void);
SPOOPY_FUNC_CORE void spoopy_audio_shutdown(void);

#ifdef __cplusplus
}
#endif
