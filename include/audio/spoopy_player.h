#pragma once

#include <spoopy.h>
#include <memory/spoopy_vector.h>
#include <audio/spoopy_stream.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct spoopy_stream_player spoopy_stream_player_t;

typedef struct spoopy_stream_player_channel {
	spoopy_stream_spec_t src_spec;
	spoopy_audio_stream_t* stream;
	void* opaque_pipe;
	float gain; // Basically the volume
	bool looping;
	bool paused;
}spoopy_stream_player_channel_t;

typedef union spoopt_audio_fmts {
	int16_t s16;
	float flt;
} spoopy_audio_fmts_t;

SPOOPY_VECTOR_LIMIT(spoopy_audio_channel_vec, spoopy_stream_player_channel_t, int)

struct spoopy_stream_player {
	spoopy_audio_channel_vec_t channels;
	spoopy_stream_spec_t dst_spec;
	float gain;
	bool paused;

	// TODO (Pick): Have the picker callback be initialized too
};

SPOOPY_FUNC_CORE bool spoopy_player_init(spoopy_stream_player_t* plr, int num_channels, const spoopy_stream_spec_t* dst_spec) SPOOPY_ATTR_NONNULL_ALL;
SPOOPY_FUNC_CORE void spoopy_player_shutdown(spoopy_stream_player_t* plr) SPOOPY_ATTR_NONNULL_ALL;
SPOOPY_FUNC_CORE size_t spoopy_player_process_channel(spoopy_stream_player_t* plr, int chan, size_t buf_size, void* buffer) SPOOPY_ATTR_NONNULL_ALL;
SPOOPY_FUNC_CORE void spoopy_player_process(spoopy_stream_player_t* plr, size_t buf_size, void* buf) SPOOPY_ATTR_NONNULL_ALL;
SPOOPY_FUNC_CORE bool spoopy_player_play(spoopy_stream_player_t* plr, int chan, spoopy_audio_stream_t* stream, bool loop, float gain, double position) SPOOPY_ATTR_NONNULL_ALL;
SPOOPY_FUNC_CORE void spoopy_player_ensure_conversion_pipe(spoopy_stream_player_t* plr, spoopy_audio_stream_t* stream, spoopy_stream_player_channel_t* pchan) SPOOPY_ATTR_NONNULL_ALL;
SPOOPY_FUNC_CORE void spoopy_player_halt(spoopy_stream_player_t* plr, int chan) SPOOPY_ATTR_NONNULL_ALL;
SPOOPY_FUNC_CORE void spoopy_player_stream_ended(spoopy_stream_player_t* plr, int chan) SPOOPY_ATTR_NONNULL_ALL;
SPOOPY_FUNC_CORE void spoopy_player_multi_audio_by_gain(spoopy_stream_spec_t spec, void* dest, const void* src, float gain);

#ifdef  __cplusplus
}
#endif
