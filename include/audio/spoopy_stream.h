#pragma once

#include <spoopy.h>
#include <spoopy_types.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct spoopy_audio_stream spoopy_audio_stream_t;

typedef enum spoopy_stream_state {
	SPOOPY_STREAM_STATE_CLOSED,   // The audio stream has shutdown or is uninitialized
	SPOOPY_STREAM_STATE_OPEN,     // The audio stream is open but paused
	SPOOPY_STREAM_STATE_STOPPED,  // The audio stream is open but stopped
	SPOOPY_STREAM_STATE_STOPPING, // The audio stream is stopping
	SPOOPY_STREAM_STATE_RUNNING,  // The audio stream is open + running
} spoopy_stream_state_t;

typedef struct spoopy_stream_callbacks {
	ssize_t (*read)(spoopy_audio_stream_t* s, size_t buf_size, void* buf);
	ssize_t (*tell)(spoopy_audio_stream_t* s);
	ssize_t (*seek)(spoopy_audio_stream_t* s, size_t pos);
	void    (*free)(spoopy_audio_stream_t* s);
} spoopy_stream_callbacks_t;

typedef struct spoopy_stream_spec {
	uint32_t sample_rate;
	uint32_t frame_rate;
	uint16_t sample_format;
	uint16_t channels;
} spoopy_stream_spec_t;

struct spoopy_audio_stream {
	spoopy_stream_callbacks_t* callbacks;
	void* buf;
	int32_t length;
	int32_t loop_start;
	spoopy_stream_spec_t spec;
};

typedef enum spoopy_audio_stream_read {
	SPOOPY_AUDIO_STREAM_READ_NONE = 0,
	SPOOPY_AUDIO_STREAM_READ_MAX_FILL = (1 << 0),
	SPOOPY_AUDIO_STREAM_READ_LOOP = (1 << 1),
} spoopy_audio_stream_read_t;


SPOOPY_FUNC_CORE ssize_t spoopy_stream_read(spoopy_audio_stream_t* stream, size_t buf_size, void* buffer, spoopy_audio_stream_read_t flags) SPOOPY_ATTR_NONNULL_ALL;
SPOOPY_FUNC_CORE ssize_t spoopy_stream_seek(spoopy_audio_stream_t* stream, int32_t pos) SPOOPY_ATTR_NONNULL_ALL;
SPOOPY_FUNC_CORE ssize_t spoopy_stream_read_into(spoopy_audio_stream_t* stream, void* opaque_stream, size_t buf_size, void* buffer, spoopy_audio_stream_read_t flags) SPOOPY_ATTR_NONNULL_ALL;
SPOOPY_FUNC_CORE ssize_t spoopy_stream_util_time_to_offset(spoopy_audio_stream_t* stream, double t) SPOOPY_ATTR_NONNULL_ALL;

SPOOPY_FUNC_CORE void* spoopy_stream_create_stream(spoopy_audio_stream_t* source, const spoopy_stream_spec_t* dest_spec);

SPOOPY_FUNC_CORE spoopy_stream_spec_t spoopy_stream_spec(uint16_t sample_format, uint16_t channels, uint32_t sample_rate);
SPOOPY_FUNC_CORE bool spoopy_stream_spec_equals(const spoopy_stream_spec_t* s1, const spoopy_stream_spec_t* s2);

#ifdef __cplusplus
}
#endif
