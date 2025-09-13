#include <internal/spoopy_internal_window.h>
#include <spoopy.h>

#include <SDL3/SDL.h>

static struct {
    SDL_Window* window;
	SDL_DisplayID* displays; // Works with all window ids since it's just an int
	int display_count;
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

void spoopy_sdl_window_create(void* raw_handle, uint32_t width, uint32_t height, bool resizeable) {
	assert(raw_handle != NULL);
	SPOOPY_LOG_INFO("Creating SDL window from Kinc handle: %p", raw_handle);

	if(video.window != NULL) {
		SPOOPY_LOG_WARN("Window already created, skipping.");
		return;
	}

	SDL_PropertiesID props = SDL_CreateProperties();

#if defined(__APPLE__)
	SDL_SetBooleanProperty(props, SDL_PROP_WINDOW_CREATE_METAL_BOOLEAN, true);
	SDL_SetPointerProperty(props, SDL_PROP_WINDOW_CREATE_COCOA_WINDOW_POINTER, raw_handle);
#endif

	SDL_SetBooleanProperty(props, SDL_PROP_WINDOW_CREATE_OPENGL_BOOLEAN, false);
	SDL_SetBooleanProperty(props, SDL_PROP_WINDOW_CREATE_RESIZABLE_BOOLEAN, resizeable);
	SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_WIDTH_NUMBER, width);
	SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_HEIGHT_NUMBER, height);

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

void spoopy_sdl_update_displays(void) {
	SDL_free(video.displays);

	int display_count = 0;
	if(!(video.displays = SDL_GetDisplays(&display_count))) {
		SPOOPY_LOG_ERROR("Failed to get display list: %s", SDL_GetError());
		video.display_count = 0;
	}
}

uint32_t spoopy_sdl_current_display(void) {
	SDL_DisplayID display_id = SDL_GetDisplayForWindow(video.window);

	if(!display_id) {
		SPOOPY_LOG_ERROR("Failed to get current display ID: %s", SDL_GetError());
		return 0;
	}

	for(int i=0; i<video.display_count; ++i) {
		if(video.displays[i] == display_id) {
			return i;
		}
	}

	assert(false && "Current display not found in display list");
	return 0;
}

bool spoopy_init_main_window(void) {
	return video.window != NULL;
}
