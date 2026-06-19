#define SPOOPY_BUILD_DEBUG
#include <spoopy_api.h>

SPOOPY_ATTR_UNUSED static const spoopy_vec2_int_t VIEWPORT = (spoopy_vec2_int_t) {
	.w = 1280,
	.h = 720,
};

static spoopy_event_handler_t* handler_ptr = NULL;

int main(int argc, char** argv) {
    SPOOPY_LOG_INFO("Audio Initialized");

	spoopy_memory_init_hooks();
	spoopy_events_init(0, &handler_ptr);

	spoopy_audio_init((spoopy_audio_request_t) {
		.format = SPOOPY_AUDIO_S16,
		.freq = 48000,
		.device_index = -1,
		.channels = 2
	}, 1, (int[]){1});

    spoopy_api_video_init(&(spoopy_video_init_params_t) {
        .title = "Test Audio",
        .width = VIEWPORT.w,
        .height = VIEWPORT.h,
		.renderer = SPOOPY_RENDERER_API_BEST_OPTION,
    });

	spoopy_audio_stream_t audio_stream = spoopy_mixer_load("test/fukkireta.opus", SPOOPY_AUDIO_FILE_FORMAT_AUTO);
	spoopy_audio_play(&audio_stream, 0, 0, true, 0);

	while(!spoopy_api_should_quit()) {
		spoopy_events_poll(handler_ptr, 0);
		spoopy_api_clear(SPOOPY_BUFFER_COLOR, SPOOPY_RGB(1.0, 1.0, 1.0), 0.0f);
		spoopy_api_swap_buffers();
	}

	spoopy_audio_shutdown();
	spoopy_api_video_shutdown();
	return 0;
}
