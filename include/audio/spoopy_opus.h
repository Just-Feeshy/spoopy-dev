#pragma once

#include <spoopy.h>
#include <audio/spoopy_stream.h>

#ifdef __cplusplus
extern "C" {
#endif

// TODO (Framework): Something needs to be done about this.
typedef struct SDL_IOStream SDL_IOStream;

// TODO (Framework):
// We need a higher-level abstraction function for this
SPOOPY_FUNC_CORE bool spoopy_opus_open(spoopy_audio_stream_t* stream, SDL_IOStream* rw, uint8_t* buf);

#ifdef __cplusplus
}
#endif
