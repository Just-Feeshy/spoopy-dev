#include "spoopy_audio_files.h"

#define AUDIO_FILE_PROBE_SIZE 64

#if SPOOPY_HAS_INCLUDE("../../spoopy_system_info.h")
#include "../../spoopy_system_info.h"
#else
#error "Could not locate (spoopy_system_info.h)"
#endif

#undef AUDIO_FILE
#define AUDIO_FILE(X) extern spoopy_audio_file_t _sp_audio_file_##X;
SPOOPY_HAS_AUDIO_FILES
#undef AUDIO_FILE

spoopy_audio_file_t* _sp_aud_files[] = {
	#define AUDIO_FILE(X) [SPOOPY_AUDIO_FILE_FORMAT_##X] = &_sp_audio_file_##X,
	SPOOPY_HAS_AUDIO_FILES
	#undef AUDIO_FILE
};

static spoopy_audio_file_t* spoopy_audio_file_probe(uint8_t buf[AUDIO_FILE_PROBE_SIZE]) {
	const size_t files_count = sizeof(_sp_aud_files)
		/ sizeof(_sp_aud_files[0]);

	for(size_t i=0; i<files_count; ++i) {
		spoopy_audio_file_t* af = _sp_aud_files[i];
		if(!af->funcs.probe) {
			continue;
		}

		bool match = af->funcs.probe(buf, AUDIO_FILE_PROBE_SIZE);
		if(match) {
			return af;
		}
	}

	return NULL;
}

static spoopy_audio_file_t* spoopy_audio_file_from_format(spoopy_audio_file_format_t file_format) {
	uint32_t index = (uint32_t)file_format;
	assert(index < SPOOPY_AUDIO_NUM_FILE_FORMATS);
	return _sp_aud_files[index];
}

bool spoopy_audio_file_open(spoopy_audio_stream_t* stream, SDL_IOStream* rw, spoopy_audio_file_format_t file_format) {
	spoopy_audio_file_t* handler = NULL;

	uint8_t buf[AUDIO_FILE_PROBE_SIZE];
	SDL_ReadIO(rw, buf, sizeof(buf));

	if(file_format == SPOOPY_AUDIO_FILE_FORMAT_AUTO) {
		handler = spoopy_audio_file_probe(buf);
	}else {
		handler = spoopy_audio_file_from_format(file_format);
	}

	if(handler == NULL) {
		SPOOPY_LOG_ERROR("No suitable audio file format handler found");
		return false;
	}

	if(SPOOPY_UNLIKELY(!handler->funcs.open)) {
		SPOOPY_LOG_ERROR("Failed to load audio using format handler: %s", handler->name);
		return false;
	}

	return handler->funcs.open(rw, stream, AUDIO_FILE_PROBE_SIZE, buf);
}
