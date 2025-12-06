#include <spoopy_api.h>
#include <spoopy_backend.h>
#include <SDL3/SDL.h>

static SDL_AtomicInt should_quit;

struct spoopy_window {
	SDL_Window* sdl_window;
};

bool spoopy_api_should_quit(void) {
	return SDL_GetAtomicInt(&should_quit);
}

void spoopy_api_request_quit(void) {
	if(SDL_CompareAndSwapAtomicInt(&should_quit, 0, 1)) {
		SPOOPY_LOG_INFO("Quit Requested");
	}
}

spoopy_window_t spoopy_api_window_create(void* raw_handle, int x, int y, int width, int height, spoopy_window_flags_t flags) {
	assert(raw_handle != NULL);
	SDL_PropertiesID props = SDL_CreateProperties();

#if defined(__APPLE__)
	SDL_SetBooleanProperty(props, SDL_PROP_WINDOW_CREATE_METAL_BOOLEAN, true);
	SDL_SetPointerProperty(props, SDL_PROP_WINDOW_CREATE_COCOA_WINDOW_POINTER, raw_handle);
#endif

	SDL_SetBooleanProperty(props, SDL_PROP_WINDOW_CREATE_RESIZABLE_BOOLEAN, (flags & SPOOPY_WINDOW_FLAG_RESIZABLE) != 0);
	SDL_SetBooleanProperty(props, SDL_PROP_WINDOW_CREATE_FULLSCREEN_BOOLEAN, (flags & SPOOPY_WINDOW_FLAG_FULLSCREEN) != 0);
	SDL_SetBooleanProperty(props, SDL_PROP_WINDOW_CREATE_BORDERLESS_BOOLEAN, (flags & SPOOPY_WINDOW_FLAG_BORDERLESS) != 0);
	SDL_SetBooleanProperty(props, SDL_PROP_WINDOW_CREATE_HIDDEN_BOOLEAN, (flags & SPOOPY_WINDOW_FLAG_HIDDEN) != 0);
	SDL_SetBooleanProperty(props, SDL_PROP_WINDOW_CREATE_VSYNC_BOOLEAN, (flags & SPOOPY_WINDOW_FLAG_VSYNC) != 0);

	SDL_SetBooleanProperty(props, SDL_PROP_WINDOW_CREATE_OPENGL_BOOLEAN, false);
	SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_WIDTH_NUMBER, width);
	SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_HEIGHT_NUMBER, height);

	SDL_Window* window = SDL_CreateWindowWithProperties(props);
	if(window == NULL) {
		SPOOPY_LOG_ERROR("Failed to create SDL window: %s", SDL_GetError());
		SDL_DestroyProperties(props);
		return 9;
	}

	return window;
}

void spoopy_api_window_show(spoopy_window_t window) {
	SDL_ShowWindow(window);
}

void spoopy_api_window_raise(spoopy_window_t window) {
	SDL_RaiseWindow(window);
}

spoopy_vec2_int_t spoopy_api_window_get_framebuffer_size(spoopy_window_t window) {
	spoopy_vec2_int_t size;
	SDL_GetWindowSize(window, &size.width, &size.height);
	return size;
}

bool spoopy_api_window_fullscreen_toggle(spoopy_window_t window) {
	return SDL_GetWindowFlags(window) & SDL_WINDOW_FULLSCREEN;
}

void spoopy_api_window_set_fullscreen(spoopy_window_t window, bool fullscreen) {
	bool sdl_flag = fullscreen ? SDL_WINDOW_FULLSCREEN : 0;

	if(!SDL_SetWindowFullscreen(window, sdl_flag)) {
		SPOOPY_LOG_ERROR("Failed to toggle fullscreen mode: %s", SDL_GetError());
	}

	SDL_RaiseWindow(window);
}

void spoopy_api_window_set_resizeable(spoopy_window_t window, bool resizeable) {
	SDL_SetWindowResizable(window, resizeable);
}

void spoopy_api_window_destroy(spoopy_window_t window) {
	if(!window) {
		SPOOPY_LOG_WARN("No window to destroy.");
		return;
	}

	SDL_DestroyWindow(window);
}

// Yea.. I know.. it's not exactly in any way part of the renderer API, but
// it's easier to just put it here since Kinc does have a nice abstraction for file IO
// Plus, it allows us to create our own file loading system even for other platforms later on.
// I mean, we could stretch the meaning of "Spoopy Renderer" to say it includes image loading since
// textures are a big part of rendering, which requires image loading
// Most will see through that bullshit anyway, but I don't think anyone will even care
bool spoopy_api_image_load_from_file(const char* path, spoopy_image_file_format_t file_format, spoopy_image_t* dst) {
	(void)path;
	(void)file_format;
	(void)dst;

	// For now though, we'll just have this be nothing
	return false;
}
