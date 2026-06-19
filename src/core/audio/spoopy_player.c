#include <audio/spoopy_player.h>
#include <audio/spoopy_audio.h>

static bool player_validate_channel(spoopy_stream_player_t* plr, int chan) {
	if(chan < 0 || chan >= plr->channels.capacity) {
		SPOOPY_LOG_ERROR("Bad Channel: %i", chan);
		return false;
	}

	return true;
}

bool spoopy_player_init(spoopy_stream_player_t* plr, int num_channels, const spoopy_stream_spec_t* dst_spec) {
	*plr = (spoopy_stream_player_t) { 0 };
	spoopy_audio_channel_vec_init(&plr->channels, num_channels);
	plr->dst_spec = *dst_spec;
	plr->gain = 1;

	for(int i=0; i<num_channels; ++i) {
		spoopy_stream_player_channel_t chan = { 0 };
		chan.gain = 1;
		chan.paused = true;
		spoopy_audio_channel_vec_add(&plr->channels, chan);
		// TODO (Pick):	Have a histroy container of sorts to be able to use for the picker callback (Just an idea)
	}

#ifdef SPOOPY_BUILD_DEBUG
	SPOOPY_LOG_INFO("Player Spec: %iHz; %i chans; format=%i",
		plr->dst_spec.sample_rate,
		plr->dst_spec.channels,
		plr->dst_spec.sample_format
	);
#endif

	return true;
}

void spoopy_player_process(spoopy_stream_player_t* plr, size_t buf_size, void* buf) {
	if(plr->paused) {
		return;
	}

	float gain = plr->gain;
	int num_channels = plr->channels.capacity;
	uint8_t bytes_per_sample = spoopy_audio_get_bits_per_sample() >> 3;

	for(int i=0; i<num_channels; ++i) {
		uint8_t staging_buffer_bytes[buf_size];
		size_t chan_bytes = spoopy_player_process_channel(plr, i, sizeof(staging_buffer_bytes), staging_buffer_bytes);

		if(chan_bytes) {
			spoopy_stream_player_channel_t* p_chan = plr->channels.data + i;
			assert(chan_bytes <= buf_size);
			assert(chan_bytes % p_chan->stream->spec.frame_rate == 0);

			float chan_gain = gain * p_chan->gain;
			uint8_t* dst_base = buf;
			uint8_t* src_base = staging_buffer_bytes;

			for(size_t i=0; i<chan_bytes; i+=bytes_per_sample) {
				spoopy_player_multi_audio_by_gain(
					p_chan->stream->spec,
					dst_base + i,
					src_base + i,
					chan_gain
				);
			}
		}
	}
}

bool spoopy_player_play(spoopy_stream_player_t* plr, int chan, spoopy_audio_stream_t* stream, bool loop, float gain, double position) {
	if(!player_validate_channel(plr, chan)) {
		return false;
	}

	if(SPOOPY_UNLIKELY(spoopy_stream_seek(stream, spoopy_stream_util_time_to_offset(stream, position)) < 0)) {
		SPOOPY_LOG_ERROR("spoopy_stream_seek() Failed!");
		goto fail;
	}

	spoopy_stream_player_channel_t* pchan = plr->channels.data + chan;

	pchan->stream = stream;
	pchan->looping = loop;
	pchan->paused = false;
	pchan->gain = gain;

	// TODO (Callback): Have a move back
	spoopy_player_ensure_conversion_pipe(plr, stream, pchan);

	return true;

fail:
	spoopy_player_halt(plr, chan);
	return false;
}

void spoopy_player_halt(spoopy_stream_player_t *plr, int chan) {
	if(!player_validate_channel(plr, chan)) {
		return;
	}

	spoopy_stream_player_channel_t* pchan = plr->channels.data + chan;
	pchan->stream = NULL;
	pchan->paused = true;
	pchan->gain = 0;

	// TODO (Pick): Have somekind of linked list for storing the channel
	// in the front of the linked list
}
