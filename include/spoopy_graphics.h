#pragma once

#include <spoopy.h>
#include <spoopy_color.h>
#include <utils/spoopy_geometry.h>
#include <memory/spoopy_vector.h>

#if SPOOPY_HAS_INCLUDE(<spoopy_renderer_config.h>)
#include <spoopy_renderer_config.h>
#else
#error "Could not find <spoopy_renderer_config.h>"
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef enum spoopy_renderer {
    SPOOPY_RENDERER_API_UNSURE = 0u,
    SPOOPY_RENDERER_API_METAL = 1u << 0,
    SPOOPY_RENDERER_API_WGPU = 1u << 1,
    SPOOPY_RENDERER_AVAILABLE =
#if defined(SPOOPY_RENDERER_METAL)
        SPOOPY_RENDERER_API_METAL |
#endif
#if defined(SPOOPY_RENDERER_WGPU)
        SPOOPY_RENDERER_API_WGPU |
#endif

        SPOOPY_RENDERER_API_UNSURE,
		SPOOPY_RENDERER_API_BEST_OPTION = SPOOPY_RENDERER_AVAILABLE & (~SPOOPY_RENDERER_AVAILABLE + 1u),
} spoopy_renderer_t;

typedef enum spoopy_buffer_kind {
	SPOOPY_BUFFER_NONE = 0,
	SPOOPY_BUFFER_COLOR = (1 << 0),
	SPOOPY_BUFFER_DEPTH = (1 << 1),

	SPOOPY_BUFFER_ALL = SPOOPY_BUFFER_COLOR | SPOOPY_BUFFER_DEPTH
} spoopy_buffer_kind_t;

typedef struct spoopy_graphics spoopy_graphics_t;

typedef struct spoopy_graphics_child {
	spoopy_renderer_t renderer;
} spoopy_graphics_child_t;


void spoopy_graphics_init(void);
spoopy_graphics_t* spoopy_graphics_new(spoopy_renderer_t renderer);
bool spoopy_graphics_set_mode(spoopy_graphics_t* graphics, void* context_view);
void spoopy_graphics_set_gamma_correction(bool gamma_correction);
bool spoopy_graphics_get_gamma_correction(void);
bool spoopy_graphics_renderer_supported(spoopy_renderer_t renderer);
bool spoopy_graphics_renderer_is_single(spoopy_renderer_t renderer);
spoopy_renderer_t spoopy_graphics_pick_renderer(spoopy_renderer_t want_mask);
spoopy_renderer_t spoopy_graphics_get_renderer(spoopy_graphics_t* graphics);
spoopy_vec2_int_t spoopy_graphics_update_present(spoopy_graphics_t* graphics);
void* spoopy_graphics_get_native_drawable(spoopy_graphics_t* graphics);

#if defined(__OBJC__)
#define SPOOPY_GRAPHICS_ASSERT_CHILD_FIRST(type)
#else
#define SPOOPY_GRAPHICS_ASSERT_CHILD_FIRST(type) \
	static_assert(offsetof(type, child) == 0, "spoopy_graphics_child_t must be first!")
#endif

#ifdef __cplusplus
}
#endif
