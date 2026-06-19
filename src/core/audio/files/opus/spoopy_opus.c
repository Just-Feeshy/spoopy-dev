#include <audio/spoopy_audio.h>
#include <audio/spoopy_stream.h>

#include <opusfile.h>
#include <SDL3/SDL_iostream.h>

#include "../spoopy_audio_files.h"
#include "../spoopy_ogg.h"

static const uint8_t opus_magic[] = { 0x4F, 0x70, 0x75, 0x73 };

static int opus_rwops_read(void* _stream, unsigned char* _ptr, int _nbytes) {
	SDL_IOStream* rw = _stream;
	return SDL_ReadIO(rw, _ptr, _nbytes);
}

static int opus_rwops_seek(void* _stream, opus_int64 _offset, int _whence) {
	SDL_IOStream* rw = _stream;
	return SDL_SeekIO(rw, _offset, _whence) < 0 ? -1 : 0;
}

static opus_int64 opus_rwops_tell(void* _stream) {
	SDL_IOStream* rw = _stream;
	return SDL_TellIO(rw);
}

static int opus_rwops_close(void* _stream) {
	SDL_IOStream* rw = _stream;
	return !SDL_CloseIO(rw) ? EOF : 0;
}

static ssize_t stream_opus_read(spoopy_audio_stream_t* s, size_t buf_size, void* buf) {
	OggOpusFile* of = s->buf; // Heh. OF

	int li = -1;
	int bytes = 0;
	ssize_t total_bytes = 0;
	size_t type_size = 1;

	uint8_t bits_per_sample = spoopy_audio_get_bits_per_sample();
	do {
		switch(bits_per_sample) {
			case 32:
				type_size = sizeof(float);
				bytes = op_read_float(of, buf, buf_size / type_size, &li);
				break;
			case 16:
				type_size = sizeof(int16_t);
				bytes = op_read(of, buf, buf_size / type_size, &li);
				break;
		}

		if(bytes > 0) {
			int channels = op_channel_count(of, li);
			total_bytes += bytes * channels * type_size;
		}
	} while(bytes == OP_HOLE);

	if(bytes < 0) {
		SPOOPY_LOG_ERROR("Opus reader failed (error %i)", bytes);
		return -1;
	}

	return total_bytes;
}

static ssize_t stream_opus_tell(spoopy_audio_stream_t* s) {
	ssize_t bytes = op_pcm_tell(s->buf);

	if(bytes < 0) {
		SPOOPY_LOG_ERROR("op_pcm_tell() - ERROR: %zi", bytes);
		return -1;
	}

	return bytes;
}

static ssize_t stream_opus_seek(spoopy_audio_stream_t* s, size_t pos) {
	ssize_t bytes = op_pcm_seek(s->buf, pos);

	if(bytes < 0) {
		SPOOPY_LOG_ERROR("op_pcm_tell() - ERROR: %zi", bytes);
		return -1;
	}

	return bytes;
}

static void stream_opus_free(spoopy_audio_stream_t* s) {
	op_free(s->buf);
}

static spoopy_stream_callbacks_t stream_opus_cb = {
	.read = stream_opus_read,
	.tell = stream_opus_tell,
	.seek = stream_opus_seek,
	.free = stream_opus_free,
};

static spoopy_stream_spec_t stream_spec(OggOpusFile* of) {
	spoopy_stream_spec_t s = { 0 };

	const OpusHead* hd = op_head(of, -1);
	if(!hd) {
		SPOOPY_LOG_ERROR("Unable to call op_head()");
		return s;
	}

	s.channels = hd->channel_count;
	s.sample_rate = hd->input_sample_rate;
	s.sample_format = spoopy_audio_get_raw_format();
	s.frame_rate = hd->channel_count * (spoopy_audio_get_bits_per_sample() / CHAR_BIT);
	return s;
}

static bool stream_opus_init(spoopy_audio_stream_t* stream, OggOpusFile* opus) {
	memset(stream, 0, sizeof(*stream));

	if(!op_seekable(opus)) {
		SPOOPY_LOG_ERROR("Opus stream is not seekable");
		return false;
	}

	ssize_t length = op_pcm_total(opus, -1);

	if(length < 0) {
		SPOOPY_LOG_ERROR("Can't determine length of Opus stream");
		return false;
	}

	if(length > INT32_MAX) {
		SPOOPY_LOG_ERROR("Opus stream is too large");
		return false;
	}

	stream->length = length;
	stream->spec = stream_spec(opus);
	stream->callbacks = &stream_opus_cb;
	stream->buf = opus;
	stream->loop_start = 0;

	return true;
}

static bool _opus_probe(const uint8_t* data, int32_t length) {
	assert(OGG_HEADER_SIZE < length);

	if(memcmp(data, ogg_fourcc, sizeof(ogg_fourcc))) {
		return false;
	}

	uint8_t segment_count = data[26];
	uint8_t page_offset = OGG_HEADER_SIZE + segment_count;
	if(page_offset + 8 > length) {
		return false;
	}

	if(memcmp(data + page_offset, opus_magic, sizeof(opus_magic))) {
		return false;
	}

	return true;
}

static bool _opus_open(SDL_IOStream* rw, spoopy_audio_stream_t* stream, uint8_t buf_size, uint8_t buf[buf_size]) {
	int error = 0;

	OggOpusFile* opus = op_open_callbacks(
		rw,
		&(OpusFileCallbacks) {
			.read  = opus_rwops_read,
			.seek  = opus_rwops_seek,
			.tell  = opus_rwops_tell,
			.close = opus_rwops_close,
		}, buf, sizeof(buf[0]) * buf_size, &error
	);

	if(opus == NULL) {
		SPOOPY_LOG_ERROR("op_open_callbacks() failed: %i", error);
		goto fail;
	}

	return stream_opus_init(stream, opus);

fail:
	SDL_SeekIO(rw, 0, SDL_IO_SEEK_SET);
	return false;
}

spoopy_audio_file_t _sp_audio_file_OPUS = {
	.name  = "opus",
	.funcs = {
		.probe = _opus_probe,
		.open  = _opus_open,
	},
};
