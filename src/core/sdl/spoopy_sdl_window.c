#include <internal/spoopy_internal_window.h>
#include <spoopy.h>

#include <SDL3/SDL.h>

static struct {
    SDL_Window* window;
} video;

void spoopy_sdl_window_init(void) {
	SDL_SetHintWithPriority(SDL_HINT_NO_SIGNAL_HANDLERS,"1",SDL_HINT_OVERRIDE);
    SDL_SetHintWithPriority(SDL_HINT_MOUSE_FOCUS_CLICKTHROUGH,"0",SDL_HINT_OVERRIDE);
    SDL_SetHintWithPriority(SDL_HINT_QUIT_ON_LAST_WINDOW_CLOSE,"1",SDL_HINT_OVERRIDE);
    SPOOPY_LOG_INFO("SDL hints set - QUIT_ON_LAST_WINDOW_CLOSE=1");

#if defined(__APPLE__)
    SDL_SetHintWithPriority(SDL_HINT_MAC_BACKGROUND_APP,"0",SDL_HINT_OVERRIDE);
    SDL_SetHintWithPriority(SDL_HINT_VIDEO_MAC_FULLSCREEN_SPACES,"0",SDL_HINT_OVERRIDE);
#endif

}

void spoopy_sdl_window_create(void* raw_handle) {
	assert(raw_handle != NULL);
	SPOOPY_LOG_INFO("Creating SDL window from Kinc handle: %p", raw_handle);

	if(video.window != NULL) {
		SPOOPY_LOG_WARN("Window already created, skipping.");
		return;
	}

	SDL_PropertiesID props = SDL_CreateProperties();

#if defined(__APPLE__)
	SDL_SetPointerProperty(props, SDL_PROP_WINDOW_CREATE_COCOA_WINDOW_POINTER, raw_handle);
#endif

	SDL_SetBooleanProperty(props, SDL_PROP_WINDOW_CREATE_OPENGL_BOOLEAN, false);
	SDL_SetBooleanProperty(props, SDL_PROP_WINDOW_CREATE_RESIZABLE_BOOLEAN, true);

	video.window = SDL_CreateWindowWithProperties(props);
	if(video.window == NULL) {
		SPOOPY_LOG_ERROR("Failed to create SDL window: %s", SDL_GetError());
		SDL_DestroyProperties(props);
		return;
	}

	SDL_DestroyProperties(props);

	if(video.window) {
		SDL_ShowWindow(video.window);
		SDL_RaiseWindow(video.window);
	}
}
