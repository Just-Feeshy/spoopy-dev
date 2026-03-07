#include <spoopy_graphics.h>

static bool __gamma_correction = false;

void spoopy_graphics_set_gamma_correction(bool gamma_correction) {
	__gamma_correction = gamma_correction;
}

bool spoopy_graphics_get_gamma_correction(void) {
	return __gamma_correction;
}

bool spoopy_graphics_renderer_supported(spoopy_renderer_t renderer) {
	return renderer & SPOOPY_RENDERER_AVAILABLE;
}

bool spoopy_graphics_renderer_is_single(spoopy_renderer_t renderer) {
    uint32_t v = (uint32_t)renderer;
    return v && ((v & (v - 1u)) == 0u);
}

spoopy_renderer_t spoopy_graphics_pick_renderer(spoopy_renderer_t want_mask) {
    uint32_t m = (uint32_t)want_mask;
    if (!m) {
        return SPOOPY_RENDERER_API_BEST_OPTION;
    }

    uint32_t lsb = m & (~m + 1u);
    return (spoopy_renderer_t)lsb;
}

spoopy_renderer_t spoopy_graphics_get_renderer(spoopy_graphics_t *graphics) {
	const spoopy_graphics_child_t* child = (const spoopy_graphics_child_t*)graphics;
	return child->renderer;
}
