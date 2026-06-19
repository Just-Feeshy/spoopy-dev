#include "../spoopy_audio_backend.h"

#include <audio/spoopy_mixer.h>
#include <utils/spoopy_misc_math.h>

#include <SDL3/SDL.h>
#include <SDL3/SDL_audio.h>

static struct {
	spoopy_mixer_t* mixer;
	SDL_AudioStream* ocean;
	SDL_AtomicU32 stream_state;
	uint8_t bits_per_sample;
	uint8_t silence; // I'm so dumb I spent 4 hours until realizing this
					 // just means, the literal silence volume value
} spoopy_audio;

static void SDLCALL on_buffer_end_callback(
	void* user_data, SDL_AudioStream* stream, int additional_amount, int total_amount) {
	(void)user_data;
	(void)total_amount;

	if(additional_amount > 0) {
		uint8_t data[additional_amount];
		memset(data, spoopy_audio.silence, additional_amount);
		spoopy_mixer_process(spoopy_audio.mixer, additional_amount, data);
		SDL_PutAudioStreamData(stream, data, additional_amount);
	}
}

static inline void lock_aud(void) {
	SDL_LockAudioStream(spoopy_audio.ocean);
}

static inline void unlock_aud(void) {
	SDL_UnlockAudioStream(spoopy_audio.ocean);
}

#define WITH_AUDIO_LOCK(...) ({ \
	lock_aud(); \
	bool _result = __VA_ARGS__; \
	unlock_aud(); \
	_result; \
})

static bool init_sdl_aud(void) {
	if(!SDL_InitSubSystem(SDL_INIT_AUDIO)) {
		SPOOPY_LOG_ERROR("SDL_InitSubSystem() - ERROR: %s\n", SDL_GetError());
		return false;
	}

	SPOOPY_LOG_INFO("Initializing SDL audio driver with %s", SDL_GetCurrentAudioDriver());
	return true;
}

static const SDL_AudioFormat formats[] = {
	[SPOOPY_AUDIO_S16] = SDL_AUDIO_S16,
	[SPOOPY_AUDIO_F32] = SDL_AUDIO_F32
};

static bool init_aud_device(SDL_AudioSpec* spec, const spoopy_audio_request_t want) {
	if(SDL_GetAtomicU32(&spoopy_audio.stream_state) != SPOOPY_STREAM_STATE_CLOSED) {
		SPOOPY_LOG_WARN("The audio stream has already be opened");
		return false;
	}

	int num_output_devices = 0;
	SDL_AudioDeviceID* devices = SDL_GetAudioPlaybackDevices(&num_output_devices);
	if(!devices || num_output_devices == 0) {
		SPOOPY_LOG_WARN("No audio devices has been found!");
		return false;
	}

	// Why `spoopy_max`?
	// Cause my dumbass when testing this and went over the array's capacity
	// I spent some good time tryna figure out where the segfault was happening
	SDL_AudioDeviceID audio_device_id = (want.device_index < 0)
								? SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK
								: devices[spoopy_max(want.device_index, num_output_devices - 1)];
	SDL_free(devices);
	devices = NULL;

	SDL_AudioSpec wspec = {
		.channels = want.channels,
		.format = formats[want.format],
		.freq = want.freq,
	};

	spoopy_audio.ocean = SDL_OpenAudioDeviceStream(audio_device_id, &wspec, &on_buffer_end_callback, NULL);
	if(!spoopy_audio.ocean) {
		SPOOPY_LOG_ERROR("SDL_OpenAudioDevice - ERROR: %s", SDL_GetError());
		return false;
	}

	*spec = wspec;
	spoopy_audio.bits_per_sample = SDL_AUDIO_BITSIZE(wspec.format);
	spoopy_audio.silence =  SDL_GetSilenceValueForFormat(wspec.format);
	SDL_SetAtomicU32(&spoopy_audio.stream_state, SPOOPY_STREAM_STATE_OPEN);
	return true;
}

static uint16_t aud_sdl_get_raw_fmt(void) {
	return spoopy_audio.mixer->spec.sample_format;
}

static bool aud_sdl_stop(void) {
	uint32_t state = SDL_GetAtomicU32(&spoopy_audio.stream_state);
	if(state != SPOOPY_STREAM_STATE_STOPPED && state != SPOOPY_STREAM_STATE_CLOSED) {
		SDL_PauseAudioDevice(SDL_GetAudioStreamDevice(spoopy_audio.ocean));
	}

	return true;
}

static bool aud_sdl_close(void) {
	if(SDL_GetAtomicU32(&spoopy_audio.stream_state) == SPOOPY_STREAM_STATE_CLOSED) {
		return false;
	}

	if(!aud_sdl_stop()) {
		return false;
	}

	SDL_DestroyAudioStream(spoopy_audio.ocean);
	SDL_SetAtomicU32(&spoopy_audio.stream_state, SPOOPY_STREAM_STATE_CLOSED);
	return true;
}

static bool aud_sdl_shutdown(void) {
	aud_sdl_close();
	SDL_QuitSubSystem(SDL_INIT_AUDIO);
	spoopy_mixer_shutdown(spoopy_audio.mixer);
	spoopy_heap_free(spoopy_audio.mixer);

	SPOOPY_LOG_SUCCESS("SDL Audio Subsystem has been successfully deinitialized");
	return true;
}

static bool aud_sdl_init(const spoopy_audio_request_t want, uint16_t num_players, int channels_per_player[num_players]) {
	memset(&spoopy_audio, 0, sizeof(spoopy_audio));

	if(!init_sdl_aud()) {
		return false;
	}

	SDL_AudioSpec aspec;

	if(!init_aud_device(&aspec, want)) {
		return false;
	}

	spoopy_audio.mixer = spoopy_heap_alloc(sizeof(spoopy_mixer_t) + num_players * sizeof(spoopy_stream_player_t));
	spoopy_stream_spec_t mspec = spoopy_stream_spec(aspec.format, want.channels, want.freq);

	if(!spoopy_mixer_init(spoopy_audio.mixer, &mspec, num_players, channels_per_player)) {
		aud_sdl_shutdown();
	}

	SDL_ResumeAudioDevice(SDL_GetAudioStreamDevice(spoopy_audio.ocean));
	SPOOPY_LOG_SUCCESS("Audio Subsystem has been successfully initialized");

	return true;
}

static bool aud_sdl_play(spoopy_audio_stream_t* stream, uint16_t player_index, int chan, bool loop, double position) {
	return WITH_AUDIO_LOCK(spoopy_player_play(
		spoopy_audio.mixer->players + player_index, chan, stream, loop, 1.0, position
	));
}

static uint8_t aud_sdl_get_bits_per_sample(void) {
	return spoopy_audio.bits_per_sample;
}

spoopy_audio_backend_t _sp_backend_sdl = {
	.name = "sdl",
	.funcs = {
		.init = aud_sdl_init,
		.play = aud_sdl_play,
		.get_bits_per_sample = aud_sdl_get_bits_per_sample,
		.get_raw_format = aud_sdl_get_raw_fmt,
		.shutdown = aud_sdl_shutdown,
	},
};
