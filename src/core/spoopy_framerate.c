#include <spoopy_framerate.h>
#include <spoopy_time.h>

void spoopy_fps_counter_update(spoopy_fps_counter_t* fps) {
	size_t update_time = spoopy_time_get();
	size_t frametime = update_time - fps->last_update_time;

	memmove(fps->frametimes, fps->frametimes + 1, (fps->log_size - 1) * sizeof(size_t));
	fps->frametimes[fps->log_size - 1] = frametime;

	size_t avg = 0;

	for(uint16_t i=0; i<fps->log_size; ++i) {
		avg += fps->frametimes[i];
	}

	fps->fps = SPOOPY_TIME_RESOLUTION / (avg / (double)fps->log_size);
	fps->frametime = avg / fps->log_size;
	fps->last_update_time = update_time;
}
