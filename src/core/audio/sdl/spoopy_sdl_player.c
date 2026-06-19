#include <audio/spoopy_player.h>

#include <SDL3/SDL_audio.h>

size_t spoopy_player_process_channel(spoopy_stream_player_t* plr, int chan, size_t buf_size, void* buffer) {
	spoopy_audio_stream_read_t r_flags = 0;
	spoopy_stream_player_channel_t* pchan = plr->channels.data + chan;

	if(pchan->paused || !pchan->stream) {
		return 0;
	}

	if(pchan->looping) {
		r_flags |= SPOOPY_AUDIO_STREAM_READ_LOOP;
	}

	uint8_t* buf = buffer;
	uint8_t* buf_end = buf + buf_size;
	spoopy_audio_stream_t* a_stream = pchan->stream;
	SDL_AudioStream* pipe = (SDL_AudioStream*)pchan->opaque_pipe;

	if(pipe) {
		do {
			uint8_t staging_buffer[buf_size];
			ssize_t read = SDL_GetAudioStreamData(pipe, buf, buf_end - buf);

			if(SPOOPY_UNLIKELY(read < 0)) {
				SPOOPY_LOG_ERROR("SDL_GetAudioStreamData() - ERROR: %s", SDL_GetError());
				break;
			}

			buf += read;

			if(buf >= buf_end) {
				break;
			}

			read = spoopy_stream_read_into(a_stream, pipe, sizeof(staging_buffer), staging_buffer, r_flags);

			if(read <= 0) {
				SDL_FlushAudioStream(pipe);

				if(SDL_GetAudioStreamAvailable(pipe) <= 0) {
					spoopy_player_halt(plr, chan);
					break;
				}
			}
		} while(buf < buf_end);
	}else {
		ssize_t read = spoopy_stream_read(a_stream, buf_size, buf, r_flags | SPOOPY_AUDIO_STREAM_READ_MAX_FILL);

		if(read > 0) {
			buf += read;
		}else {
			spoopy_player_halt(plr, chan);
		}
	}

	assert(buf <= buf_end);
	return buf_size - (buf_end - buf);
}

void spoopy_player_shutdown(spoopy_stream_player_t* plr) {
	for(int i=0; i<plr->channels.capacity; ++i) {
		spoopy_stream_player_channel_t* chan = plr->channels.data + i;
		SDL_DestroyAudioStream(chan->opaque_pipe);
	}

	spoopy_heap_free(plr);
}

void spoopy_player_ensure_conversion_pipe(spoopy_stream_player_t* plr, spoopy_audio_stream_t* stream, spoopy_stream_player_channel_t* pchan) {
	if(SPOOPY_LIKELY(spoopy_stream_spec_equals(&stream->spec, &plr->dst_spec))) {
		if(pchan->opaque_pipe) {
			SDL_DestroyAudioStream(pchan->opaque_pipe);
			pchan->opaque_pipe = NULL;
		}
	}else {
		if(pchan->opaque_pipe && SPOOPY_UNLIKELY(!spoopy_stream_spec_equals(&stream->spec, &pchan->src_spec))) {
			SDL_DestroyAudioStream(pchan->opaque_pipe);
			pchan->opaque_pipe = NULL;
		}else {
			pchan->opaque_pipe = spoopy_stream_create_stream(stream, &plr->dst_spec);
			pchan->src_spec = stream->spec;
		}
	}
}

void spoopy_player_multi_audio_by_gain(spoopy_stream_spec_t spec, void* dest, const void* src, float gain) {
	#define SAMPLE_FMT(type, sdl_type) \
		case sdl_type: {\
			type d, s; \
			memcpy(&d, dest, sizeof(d)); \
			memcpy(&s, src, sizeof(s)); \
			d += s * gain; \
			memcpy(dest, &d, sizeof(d)); \
		} break;
	switch(spec.sample_format) {
		SAMPLE_FMT(int16_t, SDL_AUDIO_S16)
		SAMPLE_FMT(float, SDL_AUDIO_F32)
	}
	#undef SAMPLE_FMT
}
