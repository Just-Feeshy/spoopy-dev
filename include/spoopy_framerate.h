#pragma once

#include <spoopy.h>
#include <memory/spoopy_memory.h>

#define SPOOPY_INIT_FPS_COUNTER(fps, alloc_type) ({ \
	const size_t _spoopy_fps_log_size = (size_t)(fps); \
	spoopy_fps_counter_t* _spoopy_fps_counter = SPOOPY_FLEX_ALLOC( \
		spoopy_fps_counter_t, \
		sizeof(size_t) * _spoopy_fps_log_size, \
		alloc_type \
	); \
	if(_spoopy_fps_counter) { \
		memset( \
			_spoopy_fps_counter, \
			0, \
			sizeof(*_spoopy_fps_counter) + sizeof(size_t) * _spoopy_fps_log_size \
		); \
		_spoopy_fps_counter->log_size = _spoopy_fps_log_size; \
	} \
	_spoopy_fps_counter; \
})

#ifdef __cplusplus
extern "C" {
#endif

typedef struct spoopy_fps_counter {
	uint16_t log_size; // Size of frametimes (Ain't no way we need more than 16k+)
	double fps; // Average fps over the last X frames
	size_t frametime; // Average frame time over the last X frames
	size_t last_update_time; // Internal; last time the average was recalculated
	size_t frametimes[]; // size = number of frames to average
} spoopy_fps_counter_t;

SPOOPY_FUNC_CORE void spoopy_fps_counter_update(spoopy_fps_counter_t* fps);

#ifdef __cplusplus
}
#endif
