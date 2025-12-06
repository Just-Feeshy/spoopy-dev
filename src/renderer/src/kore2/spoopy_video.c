#include <spoopy_api.h>
#include <spoopy_video.h>

#ifdef __APPLE__
#include <objc/objc.h>
typedef struct objc_object NSWindow;
NSWindow *kinc_get_mac_window_handle(int window_index);
#endif

#include "../../../spoopy_system_info.h"
#include "kore2.h"

// TODO (Samples): Implement multiple display support as well as multi-window support
// Might require heavy changes to the windowing system though, and also must have a sample// showing it off


static struct {
	spoopy_window_t main_window;
	spoopy_aspect_axis_t aspect_axis;
	spoopy_window_flags_t window_flags;
	const char* title;
} video;

bool video_initialized = false;

static inline int spoopy_windowpos_centered_display(uint32_t display) {
	return (int)(0x2FFF0000u | display);
}

static bool handle_kinc_window_close(void *data) {
    (void)data;
    spoopy_api_request_quit();
    return false; // Prevent immediate window close, let spoopy handle it gracefully
}

static spoopy_vec2_float_t video_get_viewport_size(spoopy_vec2_int_t framebuffer_size, spoopy_aspect_axis_t aspect_axis) {
	spoopy_vec2_float_t vp_size = {
		.w = (float)framebuffer_size.w,
		.h = (float)framebuffer_size.h,
	};

	if(framebuffer_size.h == 0) {
		return vp_size;
	}

	float fb_aspect = (float)framebuffer_size.w / (float)framebuffer_size.h;
	switch(aspect_axis) {
		case SPOOPY_ASPECT_AXIS_WIDTH:
			vp_size.h = (float)framebuffer_size.w / fb_aspect;
			break;
		case SPOOPY_ASPECT_AXIS_HEIGHT:
			vp_size.w = (float)framebuffer_size.h * fb_aspect;
			break;
		case SPOOPY_ASPECT_AXIS_NONE:
		default:
			break;
	}

	return vp_size;
}

void spoopy_video_set_viewport(uint32_t display, spoopy_aspect_axis_t aspect_axis) {
	spoopy_vec2_int_t fb;
	fb.x = kinc_window_width((int)display);
	fb.y = kinc_window_height((int)display);

	spoopy_vec2_float_t vp = video_get_viewport_size(fb, aspect_axis);
	int32_t target_w = (int32_t)vp.w;
	int32_t target_h = (int32_t)vp.h;
	int32_t pos_x = (int32_t)((fb.w - target_w) * 0.5f);
	int32_t pos_y = (int32_t)((fb.h - target_h) * 0.5f);

	kinc_window_move((int)display, pos_x, pos_y);
	kinc_window_resize((int)display, target_w, target_h);
}

static int video_flags_to_kinc_features(spoopy_window_flags_t flags) {
	int features = 0;
	if((flags & SPOOPY_WINDOW_FLAG_RESIZABLE) != 0) {
		features |= KINC_WINDOW_FEATURE_RESIZEABLE;
	}
	if((flags & SPOOPY_WINDOW_FLAG_BORDERLESS) != 0) {
		features |= KINC_WINDOW_FEATURE_BORDERLESS;
	}
	return features;
}

void video_update_mode(uint32_t display, uint32_t width, uint32_t height, spoopy_aspect_axis_t aspect_axis, spoopy_window_flags_t flags) {
	kinc_window_resize((int)display, (int)width, (int)height);
	spoopy_video_set_viewport(display, aspect_axis);
	kinc_window_change_features((int)display, video_flags_to_kinc_features(flags));
	kinc_window_mode_t mode = (flags & SPOOPY_WINDOW_FLAG_FULLSCREEN) != 0 ? KINC_WINDOW_MODE_FULLSCREEN : KINC_WINDOW_MODE_WINDOW;
	kinc_window_change_mode((int)display, mode);
}

static void video_new_window_internal(uint32_t display, uint32_t width, uint32_t height, spoopy_window_flags_t flags, bool fallback) {
	if(video.main_window != NULL) {
		spoopy_api_window_destroy(video.main_window);
		video.main_window = NULL;
	}

	void* raw_handle = NULL;
#ifdef __APPLE__
	raw_handle = kinc_get_mac_window_handle((int)display);
#endif
	if(raw_handle == NULL) {
		SPOOPY_LOG_ERROR("Failed to acquire native window handle for display %u", display);
		return;
	}

	video.main_window = spoopy_api_window_create(
		raw_handle,
		video.title,
		spoopy_windowpos_centered_display(display),
		spoopy_windowpos_centered_display(display),
		(int)width,
		(int)height,
		flags
	);

	if(video.main_window != NULL) {
		spoopy_api_window_show(video.main_window);
		spoopy_api_window_raise(video.main_window);
		video_update_mode(display, width, height, video.aspect_axis, flags);
		return;
	}

	if(fallback) {
		SPOOPY_LOG_ERROR("Failed to create window on display %u after fallback.", display);
		return;
	}

	SPOOPY_LOG_WARN("Failed to create window on display %u. Falling back to windowed mode.", display);
	video_new_window_internal(display, width, height, flags & ~SPOOPY_WINDOW_FLAG_FULLSCREEN, true);
}

static void video_new_window(uint32_t display, uint32_t width, uint32_t height, spoopy_window_flags_t flags) {
	video_new_window_internal(display, width, height, flags, false);
	if(video.main_window != NULL) {
		SPOOPY_LOG_INFO("Created new window on display %u with size %ux%u", display, width, height);
	}
}

void spoopy_video_set_mode(uint32_t display, uint32_t width, uint32_t height) {
    if(display >= kinc_count_displays()) {
        SPOOPY_LOG_WARN("Invalid display index: %u", display);
        display = 0;
    }

	if(video.main_window == NULL) {
		video_new_window(display, width, height, video.window_flags);
		return;
	}

	bool want_fullscreen = (video.window_flags & SPOOPY_WINDOW_FLAG_FULLSCREEN) != 0;
	bool is_fullscreen = spoopy_api_window_fullscreen_toggle(video.main_window);
	if(want_fullscreen != is_fullscreen) {
		spoopy_api_window_set_fullscreen(video.main_window, want_fullscreen);
	}

	bool want_resizeable = (video.window_flags & SPOOPY_WINDOW_FLAG_RESIZABLE) != 0;
	spoopy_api_window_set_resizeable(video.main_window, want_resizeable);

	video_update_mode(display, width, height, video.aspect_axis, video.window_flags);
}

void spoopy_video_init(const spoopy_video_init_params_t* params) {
    if(SPOOPY_UNLIKELY(video_initialized)) {
        SPOOPY_LOG_WARN("Video subsystem already initialized.");
        return;
    }

	video.window_flags = params->flags;
	video.aspect_axis = params->aspect_axis;
	video.title = params->title;
	__spoopy_video_internal_init_backend();
    kinc_init(params->title, params->width, params->height, NULL, NULL);
    kinc_window_set_close_callback(0, handle_kinc_window_close, NULL);

    assert(spoopy_global_context_init());

	spoopy_video_set_mode(0, params->width, params->height);

	// Setup desired slang profile and family based on the current backend
	spoopy_slang_family = 0;
	desired_slang_pf = NULL;

	// This is the overall backend selection for Spoopy Renderer
	// The specific backend driver is chosen at runtime by Kinc
#if defined(KORE_METAL)
    desired_slang_pf = "metallib_2_3";

    spoopy_slang_family  |= (1 << SLANG_METAL);
#elif defined(KORE_DIRECT3D11)
    desired_slang_pf = "sm_4_0";

    spoopy_slang_family  |= (1 << SLANG_DXBC)
                         |  (1 << SLANG_DXBC_ASM)
                         |  (1 << SLANG_DXIL)
                         |  (1 << SLANG_DXIL_ASM)
                         |  (1 << SLANG_HLSL);
#elif defined(KORE_DIRECT3D12)
    desired_slang_pf = "sm_4_0";

    spoopy_slang_family  |= (1 << SLANG_DXIL)
                         |  (1 << SLANG_DXIL_ASM)
                         |  (1 << SLANG_HLSL);
#elif defined(KORE_VULKAN)
    desired_slang_pf = "spirv_1_0";

    spoopy_slang_family  |= (1 << SLANG_SPIRV)
                         |  (1 << SLANG_SPIRV_ASM);
#endif

	SPOOPY_LOG_INFO("Video subsystem initialized");
    video_initialized = true;
}

void spoopy_video_shutdown(void) {
	if(video.main_window != NULL) {
		spoopy_api_window_destroy(video.main_window);
		video.main_window = NULL;
	}
	spoopy_shader_cleanup();
    video_initialized = false;
}
