#include "spoopy_audio_backend.h"

#if SPOOPY_HAS_INCLUDE("../../spoopy_system_info.h")
#include "../../spoopy_system_info.h"
#else
#error "Could not locate (spoopy_system_info.h)"
#endif

#undef AUDIO
#define AUDIO(x) extern spoopy_audio_backend_t _sp_backend_##x;
SPOOPY_HAS_AUDIO_BACKENDS
#undef AUDIO

spoopy_audio_backend_t* _sp_aud_backends[] = {
	#define AUDIO(x) &_sp_backend_##x,
	SPOOPY_HAS_AUDIO_BACKENDS
	#undef AUDIO
	NULL,
};

spoopy_audio_backend_t _sp_aud_backend;

static spoopy_audio_backend_t* _aud_find_backend(const char* name) {
	for(spoopy_audio_backend_t** b = _sp_aud_backends; *b; ++b) {
		if(!strcmp((*b)->name, name)) {
			return *b;
		}
	}

	SPOOPY_LOG_ERROR("Unknown audio backend: %s", name);
	return NULL;
}

static bool _aud_backend_try_init(spoopy_audio_backend_t* backend, const spoopy_audio_request_t want, uint16_t num_players, int channels_per_player[num_players]) {
	_sp_aud_backend = *backend;
#ifdef SPOOPY_BUILD_DEBUG
	SPOOPY_LOG_INFO("Trying %s", _sp_aud_backend.name);
#endif
	return _sp_aud_backend.funcs.init(want, num_players, channels_per_player);
}

bool _sp_aud_backend_init(const spoopy_audio_request_t want, uint16_t num_players, int channels_per_player[num_players]) {
	bool init = false;
	spoopy_audio_backend_t* backend = _aud_find_backend(SPOOPY_HAS_AUDIO_DEFAULT);
	if(backend) {
		init = _aud_backend_try_init(backend, want, num_players, channels_per_player);
	}

	for(spoopy_audio_backend_t** b = _sp_aud_backends; *b && !init; ++b) {
		if(*b != backend) {
			init = _aud_backend_try_init(backend, want, num_players, channels_per_player);
		}
	}

	return init;
}
