#include <spoopy_api.h>
#include <spoopy_backend.h>
#include <spoopy_graphics.h>
#include <SDL3/SDL.h>

// TODO (All tests): Have a safe way to get the cached_displays that throws a warning
// if you go out of bounds.

static struct {
	spoopy_aspect_axis_t aspect_ratio;
	bool initialized;

	SDL_AtomicInt should_quit;
	SDL_Mutex* display_mutex;

	SDL_DisplayID* cached_displays;
	int32_t cached_display_count;

	SDL_Window* primary_window; // TODO (Multi-Window): Keep this
	// TODO (Multi-Window): Have a `spoopy_window_data_t* windows` array that uses SDL_WindowID as indexes (kinda like a hash map)
	// Also, have a `SDL_Window window_prop_cache` to store properties

	spoopy_graphics_t* graphics; // TODO (Mutli-Window): Move this to `spoopy_window_data_t`

#if defined(__APPLE__)
	void* primary_view; // TODO (Multi-Window): Keep this
#endif
} app = { 0 };

static void internal_init(void) {
	spoopy_graphics_init();
	_backend_funcs.init(app.graphics);

	// TODO (States): Have `draw` state logic be initialized here
}

static void video_init_sdl(void) {
	SDL_SetHintWithPriority(SDL_HINT_FRAMEBUFFER_ACCELERATION, "0", SDL_HINT_OVERRIDE);

	if(!SDL_InitSubSystem(SDL_INIT_VIDEO)) {
		SPOOPY_LOG_ERROR("SDL_InitSubSystem() - ERROR: %s\n", SDL_GetError());
	}
}

static void new_primary_window_internal(uint32_t display, const char* title, uint32_t width, uint32_t height, spoopy_window_flags_t flags, bool fallback) {
	SDL_PropertiesID props = SDL_CreateProperties();

	if(title && *title) {
		SDL_SetStringProperty(props, SDL_PROP_WINDOW_CREATE_TITLE_STRING, title);
	}

#if defined(__APPLE__)
	SDL_SetBooleanProperty(props, SDL_PROP_WINDOW_CREATE_METAL_BOOLEAN, true);
	SDL_SetPointerProperty(props, SDL_PROP_WINDOW_CREATE_COCOA_VIEW_POINTER, app.primary_view);
#endif

	SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_WIDTH_NUMBER, width);
	SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_HEIGHT_NUMBER, height);
	SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_X_NUMBER, SDL_WINDOWPOS_CENTERED_DISPLAY(display));
	SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_Y_NUMBER, SDL_WINDOWPOS_CENTERED_DISPLAY(display));

	SDL_SetBooleanProperty(props, SDL_PROP_WINDOW_CREATE_RESIZABLE_BOOLEAN, (flags & SPOOPY_WINDOW_FLAG_RESIZABLE) != 0);
	SDL_SetBooleanProperty(props, SDL_PROP_WINDOW_CREATE_BORDERLESS_BOOLEAN, (flags & SPOOPY_WINDOW_FLAG_BORDERLESS) != 0);
	SDL_SetBooleanProperty(props, SDL_PROP_WINDOW_CREATE_FULLSCREEN_BOOLEAN, (flags & SPOOPY_WINDOW_FLAG_FULLSCREEN) != 0);
	SDL_SetBooleanProperty(props, SDL_PROP_WINDOW_CREATE_HIGH_PIXEL_DENSITY_BOOLEAN, (flags & SPOOPY_WINDOW_FLAG_HIGHDPI) != 0);

	SDL_SetBooleanProperty(props, SDL_PROP_WINDOW_CREATE_HIDDEN_BOOLEAN, false);
	SDL_SetBooleanProperty(props, SDL_PROP_WINDOW_CREATE_OPENGL_BOOLEAN, false);

	app.primary_window = SDL_CreateWindowWithProperties(props);
	SDL_DestroyProperties(props);

	if(app.primary_window) {
		SDL_ShowWindow(app.primary_window);

		// TODO (All Tests): Update video mode
		return;
	}

	if(fallback) {
		SPOOPY_LOG_ERROR("Failed to create window on display: [%s - #%u] after fallback - ERROR: %s\n"
			, spoopy_api_get_screen_name(display)
			, display
			, SDL_GetError()
		);

		return;
	}

	return new_primary_window_internal(display, title, width, height, flags & ~SPOOPY_WINDOW_FLAG_FULLSCREEN, true);
}

static void new_primary_window(const char* title, uint32_t w, uint32_t h, spoopy_window_flags_t flags, const spoopy_rec_int_t* p_rect) {
	const float scale = spoopy_api_get_screen_max_scale();
	int32_t r_screen = spoopy_api_get_screen_from_rect(p_rect);
	if(r_screen < 0) {
		r_screen = SPOOPY_PRIMARY_SCREEN_INDEX;
	}


	spoopy_vec2_int_t w_pos  = p_rect->point;
	spoopy_rec_int_t s_rect = spoopy_api_screen_get_usable_rect(r_screen);
	if(!spoopy_rec_int_equal(s_rect, (spoopy_rec_int_t){ 0 })) {
		spoopy_vec2_int_t hi = s_rect.point;
		spoopy_vec2_int_add(&hi, s_rect.size);

		spoopy_vec2_int_t p_size_third = p_rect->size;
		spoopy_vec2_int_div(&p_size_third, 3);
		spoopy_vec2_int_sub(&hi, p_size_third);

		spoopy_vec2_int_clamp(&w_pos, s_rect.point, hi);
	}

	const int win_w_pts = spoopy_max(1, (int)SDL_lround(w_pos.x / scale));
	const int win_h_pts = spoopy_max(1, (int)SDL_lround(w_pos.y / scale));

#if defined(__APPLE__)
	app.primary_view = SDL_Metal_CreateView(app.primary_view);
	assert(app.primary_view != NULL);
#endif

	new_primary_window_internal(r_screen, title, (uint32_t)win_w_pts, (uint32_t)win_h_pts, flags, false);

	SPOOPY_LOG_INFO("Create a new window: %ix%i, on display #%i %s\n", win_w_pts, win_h_pts, r_screen, spoopy_api_get_screen_name(r_screen));
	SDL_RaiseWindow(app.primary_window);
}

bool spoopy_api_should_quit(void) {
	return SDL_GetAtomicInt(&app.should_quit);
}

void spoopy_api_request_quit(void) {
	if(SDL_CompareAndSwapAtomicInt(&app.should_quit, 0, 1)) {
		SPOOPY_LOG_INFO("Quit Requested");
	}
}

int32_t spoopy_api_get_screen_count(void) {
	return app.cached_display_count;
}

void spoopy_api_refresh_screens(void) {
	SDL_LockMutex(app.display_mutex);
	SDL_free(app.cached_displays);

	int screen_count = app.cached_display_count;
	if(!(app.cached_displays = SDL_GetDisplays(&screen_count))) {
		SPOOPY_LOG_ERROR("SDL_InitSubSystem() - ERROR: %s\n", SDL_GetError());
		app.cached_display_count = 0;
	}

	SDL_UnlockMutex(app.display_mutex);
}

void spoopy_api_video_init(const spoopy_video_init_params_t* params) {
	if(app.initialized) {
		SPOOPY_LOG_WARN("`spoopy_api_video_init()` has already been called!");
		return;
	}

	video_init_sdl();

	const char *driver = SDL_GetCurrentVideoDriver();
	SPOOPY_LOG_INFO("Using driver '%s'", driver);

	app.initialized = true;
	app.aspect_ratio = params->aspect_axis;
	app.display_mutex = SDL_CreateMutex();

	internal_init();

	uint32_t w = spoopy_min(params->width, 0);
	uint32_t h = spoopy_min(params->height, 0);

	app.graphics = spoopy_graphics_new(params->renderer);
	spoopy_api_refresh_screens();
}

// This allows us to create our own file loading system even for other platforms later on.
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

void spoopy_api_video_shutdown(void) {
}

int32_t spoopy_api_get_screen_from_rect(const spoopy_rec_int_t* rect) {
	const SDL_Rect r = {
		.x = rect->point.x,
		.y = rect->point.y,
		.w = rect->size.w,
		.h = rect->size.h
	};

	int32_t nearest_area = 0;
	int32_t pos_screen = -1;

	SDL_LockMutex(app.display_mutex);

	SDL_DisplayID* displays = app.cached_displays;

	for(int32_t i=0; i<spoopy_api_get_screen_count(); i++) {
		SDL_Rect db;
		if(!SDL_GetDisplayBounds(displays[i], &db)) {
			continue;
		}

		SDL_Rect inter;
		if(SDL_GetRectIntersection(&db, &r, &inter)) {
			const int area = inter.w * inter.h;
			if(area > nearest_area) {
				pos_screen = i;
				nearest_area = area;
			}
		}
	}

	SDL_UnlockMutex(app.display_mutex);
	return pos_screen;
}

const char* spoopy_api_get_screen_name(uint32_t screen_index) {
	SDL_LockMutex(app.display_mutex);
	const char* name = SDL_GetDisplayName(app.cached_displays[screen_index]);

	if(name == NULL) {
		SPOOPY_LOG_WARN("SDL_GetDisplayName() - WARN: %s\n", SDL_GetError());
		name = "Unknown";
	}

	SDL_UnlockMutex(app.display_mutex);
	return name;
}

float spoopy_api_get_screen_max_scale(void) {
	SDL_LockMutex(app.display_mutex);

	int32_t count = app.cached_display_count;
	SDL_DisplayID* displays = app.cached_displays;
	float max_scale = 1.0f;

	if(!displays || count <= 0) {
		goto return_max_scale;
	}

	for(int32_t i=0; i<count; i++) {
		const float s = SDL_GetDisplayContentScale(displays[i]);
		if(s > max_scale) {
			max_scale = s;
		}
	}

return_max_scale:
	SDL_UnlockMutex(app.display_mutex);
	return max_scale;
}

spoopy_rec_int_t spoopy_api_screen_get_usable_rect(int32_t screen_index) {
	SDL_LockMutex(app.display_mutex);

	spoopy_rec_int_t rec2 = { 0 };

	if(screen_index < 0) {
		screen_index = SPOOPY_PRIMARY_SCREEN_INDEX;
	}

	if(screen_index >= app.cached_display_count) {
		goto got_usable_rect;
	}

	SDL_DisplayID display = app.cached_displays[screen_index];
	SDL_Rect sdl_rec2 = { 0 };

	if(!SDL_GetDisplayUsableBounds(display, &sdl_rec2)) {
		goto got_usable_rect;
	}

	spoopy_vec2_int_t pos = { .x = sdl_rec2.x, .y = sdl_rec2.y };
	spoopy_vec2_int_t size = { .x = sdl_rec2.w, .y = sdl_rec2.h };
	rec2 = (spoopy_rec_int_t){ .point = pos, .size = size };

got_usable_rect:
	SDL_UnlockMutex(app.display_mutex);
	return rec2;
}

spoopy_renderer_t spoopy_api_get_renderer(void) {
	return spoopy_graphics_get_renderer(app.graphics);
}
