#include <internal/spoopy_internal_window.h>
#include <spoopy.h>

static struct {
    SDL_Window* window;
} video;

void spoopy_window_create(void* raw_handle, uint32_t width, uint32_t height, bool resizeable) {
	assert(raw_handle != NULL);
	SPOOPY_LOG_INFO("Creating SDL window from Kinc handle: %p", raw_handle);

	if(video.window != NULL) {
		SPOOPY_LOG_WARN("Window already created, destroying existing window.");
		SDL_DestroyWindow(video.window);
		video.window = NULL;
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

	if(!video.window) {
		SPOOPY_LOG_ERROR("Failed to create SDL window.");
		return;
	}

	SDL_ShowWindow(video.window);
	SDL_RaiseWindow(video.window);
}

void spoopy_window_destroy(void) {
	if(!video.window) {
		SPOOPY_LOG_WARN("No window to destroy.");
		return;
	}
	SDL_DestroyWindow(video.window);
	video.window = NULL;
}

bool spoopy_init_main_window(void) {
	return video.window != NULL;
}

static spoopy_vec2_int_t spoopy_window_get_framebuffer_size(void) {
	spoopy_vec2_int_t size;
	SDL_GetWindowSizeInPixels(video.window, &size.x, &size.y);
	return size;
}

static spoopy_vec2_float_t spoopy_window_get_viewport(spoopy_vec2_int_t viewport, spoopy_aspect_axis_t aspect_axis) {
	spoopy_vec2_int_t size = spoopy_window_get_framebuffer_size();

	if(viewport.width != 0 && viewport.height != 0) {
		size = viewport;
	}

	spoopy_vec2_float_t fb_size;
	fb_size.x = (float)size.width;
	fb_size.y = (float)size.height;
	float aspect_ratio = fb_size.x / fb_size.y;

	if(aspect_axis == SPOOPY_ASPECT_AXIS_WIDTH) {
		fb_size.y = fb_size.x / aspect_ratio;
	} else {
		fb_size.x = fb_size.y * aspect_ratio;
	}

	return fb_size;
}
