#pragma once

#include <spoopy.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    SPOOPY_RENDERER_API_UNSURE = 0u,
    SPOOPY_RENDERER_API_METAL = 1u << 0,
    SPOOPY_RENDERER_API_D3D11 = 1u << 1,
    SPOOPY_RENDERER_API_WGPU = 1u << 2,
    SPOOPY_RENDERER_AVAILABLE =
#if defined(SPOOPY_RENDERER_METAL)
        SPOOPY_RENDERER_API_METAL |
#endif
#if defined(SPOOPY_RENDERER_D3D11)
        SPOOPY_RENDERER_API_D3D11 |
#endif
#if defined(SPOOPY_RENDERER_WGPU)
        SPOOPY_RENDERER_API_WGPU |
#endif

        SPOOPY_RENDERER_API_UNSURE,
		SPOOPY_RENDERER_API_BEST_OPTION = SPOOPY_RENDERER_AVAILABLE & (~SPOOPY_RENDERER_AVAILABLE + 1u),
} spoopy_renderer_t;

typedef struct spoopy_graphics_child {
	spoopy_renderer_t renderer;
} spoopy_graphics_child_t;

typedef struct spoopy_graphics spoopy_graphics_t;

void spoopy_graphics_init(void);
spoopy_graphics_t* spoopy_graphics_new(spoopy_renderer_t renderer);
bool spoopy_graphics_set_mode(spoopy_graphics_t* graphics, void* context_view);
void spoopy_graphics_set_gamma_correction(bool gamma_correction);
bool spoopy_graphics_get_gamma_correction(void);
bool spoopy_graphics_renderer_supported(spoopy_renderer_t renderer);
bool spoopy_graphics_renderer_is_single(spoopy_renderer_t renderer);
spoopy_renderer_t spoopy_graphics_pick_renderer(spoopy_renderer_t want_mask);
spoopy_renderer_t spoopy_graphics_get_renderer(spoopy_graphics_t* graphics);

#ifdef SPOOPY_GRAPHICS_IMPL
static_assert(offsetof(spoopy_graphics_t, child) == 0, "spoopy_graphics_child_t must be first!");
#undef SPOOPY_GRAPHICS_IMPL
#endif

#ifdef __cplusplus
}
#endif
