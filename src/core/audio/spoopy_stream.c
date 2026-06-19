#include <audio/spoopy_stream.h>
#include <audio/spoopy_audio.h>
#include <SDL3/SDL_iostream.h>

#define CBS(stream) (*((stream)->callbacks))
#define CB(stream, func) CBS(stream).func

// A lot has been adapted from Taisei
// https://github.com/taisei-project/taisei/blob/master/src/audio/stream/stream.c

ssize_t spoopy_stream_read(spoopy_audio_stream_t* stream, size_t buf_size, void* buffer, spoopy_audio_stream_read_t flags) {
	if(flags & SPOOPY_AUDIO_STREAM_READ_MAX_FILL) {
		flags &= ~SPOOPY_AUDIO_STREAM_READ_MAX_FILL;
		char* cbuf = buffer;
		char* end = cbuf + buf_size;
		ssize_t read;

		while(end - cbuf > 0 && (read = spoopy_stream_read(stream, end - cbuf, cbuf, flags)) > 0) {
			cbuf += read;
		}

		return cbuf - (char*)buffer;
	}

	if(flags & SPOOPY_AUDIO_STREAM_READ_LOOP) {
		ssize_t read = CB(stream, read)(stream, buf_size, buffer);

		if(SPOOPY_UNLIKELY(read == 0)) {
			ssize_t loop_start = spoopy_max(stream->loop_start, 0);

			if(SPOOPY_UNLIKELY(spoopy_stream_seek(stream, loop_start) < 0)) {
				return -1;
					}

			return CB(stream, read)(stream, buf_size, buffer);
		}

		return read;
	}

	return CB(stream, read)(stream, buf_size, buffer);
}

ssize_t spoopy_stream_seek(spoopy_audio_stream_t *stream, int32_t pos) {
	if(stream->length >= 0 && pos >= stream->length) {
		SPOOPY_LOG_ERROR("Position %i out of range", pos);
		return -1;
	}

	if(SPOOPY_LIKELY(CBS(stream).seek)) {
		return CB(stream, seek)(stream, pos);
	}

	SPOOPY_LOG_ERROR("Not Implemented");
	return -1;
}

ssize_t spoopy_stream_util_time_to_offset(spoopy_audio_stream_t *stream, double t) {
	return t * stream->spec.sample_rate;
}

spoopy_stream_spec_t spoopy_stream_spec(uint16_t sample_format, uint16_t channels, uint32_t sample_rate) {
	spoopy_stream_spec_t s;
	s.sample_format = sample_format;
	s.channels = channels;
	s.sample_rate = sample_rate;
	s.frame_rate = channels * (spoopy_audio_get_bits_per_sample() >> 3);

	assert(s.frame_rate > 0);
	return s;
}

bool spoopy_stream_spec_equals(const spoopy_stream_spec_t* s1, const spoopy_stream_spec_t* s2) {
	return
		s1->channels == s2->channels &&
		s1->sample_rate == s2->sample_rate &&
		s1->sample_format == s2->sample_format;
}
