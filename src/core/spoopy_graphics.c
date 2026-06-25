#include <spoopy.h>

#if SPOOPY_HAS_INCLUDE("../spoopy_system_info.h")
#include "../spoopy_system_info.h"
#endif

#include <spoopy_graphics.h>

static bool __gamma_correction = false;

static const spoopy_renderer_t _spoopy_renderers[] = {
	SPOOPY_RENDERER_API_AVAILABLE_LIST
	SPOOPY_RENDERER_API_UNSURE,
};

void spoopy_graphics_set_gamma_correction(bool gamma_correction) {
	__gamma_correction = gamma_correction;
}

bool spoopy_graphics_get_gamma_correction(void) {
	return __gamma_correction;
}

bool spoopy_graphics_renderer_supported(spoopy_renderer_t renderer) {
	for(const spoopy_renderer_t* available = _spoopy_renderers; *available; ++available) {
		if(renderer & *available) {
			return true;
		}
	}

	return false;
}

bool spoopy_graphics_renderer_is_single(spoopy_renderer_t renderer) {
    uint32_t v = (uint32_t)renderer;
    return v && ((v & (v - 1u)) == 0u);
}

spoopy_renderer_t spoopy_graphics_pick_renderer(spoopy_renderer_t want_mask) {
	if(!want_mask) {
		return _spoopy_renderers[0];
	}

	for(const spoopy_renderer_t* available = _spoopy_renderers; *available; ++available) {
		if(want_mask & *available) {
			return *available;
		}
	}

	return SPOOPY_RENDERER_API_UNSURE;
}

spoopy_renderer_t spoopy_graphics_get_renderer(spoopy_graphics_t *graphics) {
	const spoopy_graphics_child_t* child = (const spoopy_graphics_child_t*)graphics;
	return child->renderer;
}
