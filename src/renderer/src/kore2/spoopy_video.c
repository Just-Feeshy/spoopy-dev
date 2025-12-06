#include <spoopy_api.h>

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
} video;

bool video_initialized = false;

static bool handle_kinc_window_close(void *data) {
    (void)data;
    spoopy_api_request_quit();
    return false; // Prevent immediate window close, let spoopy handle it gracefully
}

static spoopy_vec2_float_t video_get_viewport_size(spoopy_vec2_int_t framebuffer_size) {
	spoopy_vec2_float_t vp_size;

	float fb_aspect = (float)framebuffer_size.w / (float)framebuffer_size.h;

	switch(video.aspect_axis) {
		case SPOOPY_ASPECT_AXIS_WIDTH:
			vp_size.w = (float)framebuffer_size.w;
			vp_size.h = (float)framebuffer_size.w / fb_aspect;
			break;
		case SPOOPY_ASPECT_AXIS_HEIGHT:
			vp_size.h = (float)framebuffer_size.h;
			vp_size.w = (float)framebuffer_size.h * fb_aspect;
			break;
		case SPOOPY_ASPECT_AXIS_NONE:
		default:
			vp_size.w = (float)framebuffer_size.w;
			vp_size.h = (float)framebuffer_size.h;
			break;
	}

	return vp_size;
}

static void video_set_viewport(uint32_t display) {
	spoopy_rec2_int_t vp;

	{
		spoopt_vec2_int_t fb = spoopy_api_window_get_framebuffer_size(video.main_window);
		vp.w = fb.w;
		vp.h = fb.h;

		spoopy_vec2_float_t vp_size = video_get_viewport_size(vp, video.aspect_axis);
		vp.x = (int32_t)((fb.w - (int32_t)vp_size.w) * 0.5f);
		vp.y = (int32_t)((fb.h - (int32_t)vp_size.h) * 0.5f);
	}

	kinc_window_move(display, vp.x, vp.y);
	kinc_window_resize(display, vp.w, vp.h);
}

static void video_update_mode(uint32_t display, uint32_t width, uint32_t height, spoopy_aspect_axis_t aspect_axis, bool resizeable) {
	video_set_viewport(display, video.aspect_axis);
	kinc_window_change_mode(display, width, height, resizeable);
}

static void video_new_window_interal(uint32_t display, uint32_t width, uint32_t height, bool fallback) {
	if(video.main_window) {
		spoopy_api_window_destroy(video.main_window);
		video.main_window = NULL;
	}

	void* raw_handle = NULL;

#ifdef __APPLE__
	raw_handle = kinc_get_mac_window_handle(display);
#endif

	video.main_window = spoopy_api_window_create(
		raw_handle,
		SDL_WINDOWPOS_CENTERED_DISPLAY(display),
		SDL_WINDOWPOS_CENTERED_DISPLAY(display),
		width,
		height,
		video.window_flags
	);

	if(video.main_window) {
		kinc_window_show(display);
		video_update_mode(display, width, height, (video.window_flags & SPOOPY_WINDOW_FLAG_RESIZABLE) != 0);
		return;
	}

	if(fallback) {
		SPOOPY_LOG_ERROR("Failed to create window on display %u. Falling back to default display.", display);
		return;
	}

	video_new_window_internal(display, width, height, video.window_flags & ~SPOOPY_WINDOW_FLAG_FULLSCREEN, true);
}

static void video_new_window(uint32_t display, uint32_t width, uint32_t height) {
	video_new_window_interal(display, width, height, video.window_flags, false);
	SPOOPY_LOG_INFO("Created new window on display %u with size %ux%u", display, width, height);
}

void video_set_mode(uint32_t display, uint32_t width, uint32_t height) {
    if(display >= kinc_count_displays()) {
        SPOOPY_LOG_WARN("Invalid display index: %u", display);
        display = 0;
    }

	if(!video.main_window) {
		video_new_window(display, width, height, video.window_flags);
		return;
	}

	bool resizeable_changed = !(((video.window_flags & SPOOPY_WINDOW_FLAG_RESIZABLE) != 0) && spoopy_api_window_fullscreen_toggle(video.main_window));

	if(resizeable_changed) {
		video_new_window(display, width, height, video.window_flags);
		return;
	}

	spoopy_api_window_set_fullscreen(video.main_window, (video.window_flags & SPOOPY_WINDOW_FLAG_FULLSCREEN) != 0);
	spoopy_api_window_set_resizeable(video.main_window, (video.window_flags & SPOOPY_WINDOW_FLAG_RESIZABLE) != 0);
}

void spoopy_video_init(const spoopy_video_init_params_t* params) {
    if(SPOOPY_UNLIKELY(video_initialized)) {
        SPOOPY_LOG_WARN("Video subsystem already initialized.");
        return;
    }

	__spoopy_video_internal_init_backend();
    kinc_init(params->title, params->width, params->height, NULL, NULL);
    kinc_window_set_close_callback(0, handle_kinc_window_close, NULL);

    assert(spoopy_global_context_init());

	video_set_mode(0, params->width, params->height, true);

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
	spoopy_shader_cleanup();
	spoopy_window_destroy();
    video_initialized = false;
}
