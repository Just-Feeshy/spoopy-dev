#include <spoopy.h>
#include <spoopy_log.h>

#ifdef SPOOPY_USING_KORE2

#include <Apple/Sources/kinc/backend/http.m.h>
#include <Apple/Sources/kinc/backend/system.m.h>

#include <kinc/threads/thread.h>
#include <kinc/graphics4/texture.h>
#include <kinc/video.h>


/ ** Empty functions to satisfy the linker ** /

void kinc_threads_init(void) {
	SPOOPY_LOG_ERROR("kinc_threads_init is not supported via the Spoopy Environment, please use `spoopy_{type}_thread_init` instead.");
}

void kinc_threads_quit(void) {
	SPOOPY_LOG_ERROR("kinc_threads_quit is not supported via the Spoopy Environment, please use `spoopy_{type}_thread_shutdown` instead.");
}

void kinc_thread_init(kinc_thread_t *thread, void (*func)(void* param), void* param) {
	SPOOPY_LOG_ERROR("kinc_thread_init is not supported via the Spoopy Environment, please use `spoopy_{type}_thread_create` instead.");
}

void kinc_thread_wait_and_destroy(kinc_thread_t *thread) {
	SPOOPY_LOG_ERROR("kinc_thread_wait_and_destroy is not supported via the Spoopy Environment, please use `spoopy_{type}_thread_wait` instead.");
}

void kinc_thread_set_name(kinc_thread_t *thread, const char *name) {
	SPOOPY_LOG_ERROR("kinc_thread_set_name is not supported via the Spoopy Environment.");
}

void kinc_thread_sleep(int milliseconds) {
	SPOOPY_LOG_ERROR("kinc_thread_sleep is not supported via the Spoopy Environment.");
}


const char** kinc_video_formats(void) {
	SPOOPY_LOG_ERROR("kinc_video_formats is not supported via the Spoopy Environment.");
	return NULL;
}

void kinc_video_init(kinc_video_t* video, const char* filename) {
	SPOOPY_LOG_ERROR("kinc_video_init is not supported via the Spoopy Environment.");
}

void kinc_video_destroy(kinc_video_t* video) {
	SPOOPY_LOG_ERROR("kinc_video_destroy is not supported via the Spoopy Environment.");
}

void kinc_video_play(kinc_video_t* video, bool loop) {
	SPOOPY_LOG_ERROR("kinc_video_play is not supported via the Spoopy Environment.");
}

void kinc_video_pause(kinc_video_t* video) {
	SPOOPY_LOG_ERROR("kinc_video_pause is not supported via the Spoopy Environment.");
}

void kinc_video_stop(kinc_video_t* video) {
	SPOOPY_LOG_ERROR("kinc_video_stop is not supported via the Spoopy Environment.");
}

int kinc_video_width(kinc_video_t* video) {
	SPOOPY_LOG_ERROR("kinc_video_width is not supported via the Spoopy Environment.");
	return 0;
}
int kinc_video_height(kinc_video_t* video) {
	SPOOPY_LOG_ERROR("kinc_video_height is not supported via the Spoopy Environment.");
	return 0;
}

kinc_g4_texture_t* kinc_video_current_image(kinc_video_t* video) {
SPOOPY_LOG_ERROR("kinc_video_current_image is not supported via the Spoopy Environment.");
	return NULL;
}

double kinc_video_duration(kinc_video_t* video) {
	SPOOPY_LOG_ERROR("kinc_video_duration is not supported via the Spoopy Environment.");
	return 0.0;
}

double kinc_video_position(kinc_video_t* video) {
	SPOOPY_LOG_ERROR("kinc_video_position is not supported via the Spoopy Environment.");
	return 0.0;
}

bool kinc_video_finished(kinc_video_t* video) {
	SPOOPY_LOG_ERROR("kinc_video_finished is not supported via the Spoopy Environment.");
	return false;
}

bool kinc_video_paused(kinc_video_t* video) {
	SPOOPY_LOG_ERROR("kinc_video_paused is not supported via the Spoopy Environment.");
	return false;
}

void kinc_video_update(kinc_video_t* video) {
	SPOOPY_LOG_ERROR("kinc_video_update is not supported via the Spoopy Environment.");
}

#endif
