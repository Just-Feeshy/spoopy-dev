#include <spoopy_video.h>
#include <memory/spoopy_mem_vector.h>
#include <spoopy_log.h>
#include <SDL3/SDL.h>

static struct {
	spoopy_mem_vector_t windows;
} video_manager;

static SDL_Window* video_new_window_internal(uint32_t display, uint32_t width, uint32_t height, spoopy_window_flags_t flags, bool fallback) {
	SDL_Window* window;
	SDL_PropertiesID props = SDL_CreateProperties();

	if(title && *title) {
		SDL_SetStringProperty(props, SDL_PROP_WINDOW_CREATE_TITLE_STRING, title);
	}

#if defined(__APPLE__)
	SDL_SetBooleanProperty(props, SDL_PROP_WINDOW_CREATE_METAL_BOOLEAN, true);
#endif

	SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_WIDTH_NUMBER, width);
	SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_HEIGHT_NUMBER, height);
	SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_DISPLAY_INDEX_NUMBER, display);
	SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_POSITION_X_NUMBER, SDL_WINDOWPOS_CENTERED_DISPLAY(display));
	SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_POSITION_Y_NUMBER, SDL_WINDOWPOS_CENTERED_DISPLAY(display));

	SDL_SetBooleanProperty(props, SDL_PROP_WINDOW_CREATE_RESIZABLE_BOOLEAN, (flags & SPOOPY_WINDOW_FLAG_RESIZABLE) != 0);
	SDL_SetBooleanProperty(props, SDL_PROP_WINDOW_CREATE_BORDERLESS_BOOLEAN, (flags & SPOOPY_WINDOW_FLAG_BORDERLESS) != 0);
	SDL_SetBooleanProperty(props, SDL_PROP_WINDOW_CREATE_VSYNC_BOOLEAN, (flags & SPOOPY_WINDOW_FLAG_VSYNC) != 0);
	SDL_SetBooleanProperty(props, SDL_PROP_WINDOW_CREATE_FULLSCREEN_BOOLEAN, (flags & SPOOPY_WINDOW_FLAG_FULLSCREEN) != 0);
	SDL_SetBooleanProperty(props, SDL_PROP_WINDOW_CREATE_HIGHDPI_BOOLEAN, (flags & SPOOPY_WINDOW_FLAG_HIGHDPI) != 0);

	SDL_SetBooleanProperty(props, SDL_PROP_WINDOW_CREATE_HIDDEN_BOOLEAN, false);
	SDL_SetBooleanProperty(props, SDL_PROP_WINDOW_CREATE_OPENGL_BOOLEAN, false);

	window = SDL_CreateWindowWithProperties(props);
	SDL_DestroyProperties(props);

	if(window) {
		SDL_ShowWindow(window);

		// TODO (Samples): Update video mode
		return window;
	}

	if(fallback) {
		SPOOPY_LOG_ERROR("Failed to create window on display %u after fallback.", display);
		return NULL;
	}

	return video_new_window_internal(display, width, height, flags & ~SPOOPY_WINDOW_FLAG_FULLSCREEN, true);
}

void spoopy_window_create(uint32_t display, uint32_t width, uint32_t height, spoopy_window_flags_t flags) {
	SDL_Window* window = video_new_window_internal(display, width, height, flags, false);


}

void __spoopy_video_internal_init_backend(void) {
	SDL_SetHintWithPriority(SDL_HINT_NO_SIGNAL_HANDLERS,"1",SDL_HINT_OVERRIDE);
    SDL_SetHintWithPriority(SDL_HINT_MOUSE_FOCUS_CLICKTHROUGH,"0",SDL_HINT_OVERRIDE);
    SDL_SetHintWithPriority(SDL_HINT_QUIT_ON_LAST_WINDOW_CLOSE,"1",SDL_HINT_OVERRIDE);
    SPOOPY_LOG_INFO("SDL hints set - QUIT_ON_LAST_WINDOW_CLOSE=1");

#if defined(__APPLE__)
    SDL_SetHintWithPriority(SDL_HINT_MAC_BACKGROUND_APP,"0",SDL_HINT_OVERRIDE);
    SDL_SetHintWithPriority(SDL_HINT_VIDEO_MAC_FULLSCREEN_SPACES,"0",SDL_HINT_OVERRIDE);
#endif
}

static void video_sdl_init(void) {
	SDL_SetHintWithPriority(SDL_HINT_FRAMEBUFFER_ACCELERATION, "0", SDL_HINT_OVERRIDE);

	if(!SDL_InitSubSystem(SDL_INIT_VIDEO)) {
		SPOOPY_LOG_ERROR("Failed to initialize SDL Video subsystem: %s", SDL_GetError());
	}
}

void spoopy_video_init(void) {
	video_sdl_init();

	const char *driver = SDL_GetCurrentVideoDriver();
	SPOOPY_LOG_INFO("Using SDL Video Driver: %s", driver);

		
}
