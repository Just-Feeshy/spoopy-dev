#include <audio/spoopy_stream.h>

#include <SDL3/SDL_audio.h>

ssize_t spoopy_stream_read_into(spoopy_audio_stream_t* stream, void* opaque_stream, size_t buf_size, void* buffer, spoopy_audio_stream_read_t flags) {
	char* buf = buffer;
	ssize_t read_size = spoopy_stream_read(stream, buf_size, buffer, flags);
	SDL_AudioStream* s = (SDL_AudioStream*)opaque_stream;

	if(SPOOPY_LIKELY(read_size > 0)) {
		if(SPOOPY_UNLIKELY(!SDL_PutAudioStreamData(s, buf, read_size))) {
			SPOOPY_LOG_ERROR("SDL_PutAudioStreamData - ERROR: %s", SDL_GetError());
			return -1;
		}
	}

	return read_size;
}

void* spoopy_stream_create_stream(spoopy_audio_stream_t* source, const spoopy_stream_spec_t* dest_spec) {
	return SDL_CreateAudioStream(&(SDL_AudioSpec){
		.channels = source->spec.channels,
		.format = source->spec.sample_format,
		.freq = source->spec.sample_rate,
	}, &(SDL_AudioSpec) {
		.channels = dest_spec->channels,
		.format = dest_spec->sample_format,
		.freq = dest_spec->sample_rate,
	});
}
