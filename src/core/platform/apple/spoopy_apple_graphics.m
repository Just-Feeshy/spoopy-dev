#include <spoopy_graphics.h>
#include <spoopy_log.h>
#include <memory/spoopy_memory.h>

#include <renderer/spoopy_metal.h>

#import <Foundation/Foundation.h>
#import <CoreGraphics/CoreGraphics.h>
#import <QuartzCore/CAMetalLayer.h>
#import <Metal/Metal.h>

#include <SDL3/SDL.h>
#include <SDL3/SDL_metal.h>

struct spoopy_graphics {
	spoopy_graphics_child_t child;

	CAMetalLayer* metal_layer;
	id<CAMetalDrawable> active_drawable;
};

SPOOPY_GRAPHICS_ASSERT_CHILD_FIRST(spoopy_graphics_t);

static inline bool _spoopy_mtl_set_mode(spoopy_graphics_t* graphics, void* context_view) {
	@autoreleasepool {
		SDL_MetalView view = (SDL_MetalView)context_view;
		graphics->metal_layer = (CAMetalLayer*)SDL_Metal_GetLayer(view);

		graphics->metal_layer.device = spoopy_metal_init();
		graphics->metal_layer.pixelFormat = spoopy_graphics_get_gamma_correction()
			? MTLPixelFormatBGRA8Unorm_sRGB
			: MTLPixelFormatBGRA8Unorm;

		graphics->metal_layer.framebufferOnly = YES;

		return true;
	}
}

static inline const spoopy_vec2_int_t _spoopy_mtl_update_present(spoopy_graphics_t* graphics) {
	// Don't set drawableSize manually - let the system handle it like the reference
	graphics->active_drawable = [graphics->metal_layer nextDrawable];

	if (!graphics->active_drawable) {
		SPOOPY_LOG_WARN("nextDrawable returned nil!");
		return (spoopy_vec2_int_t) { .x = 0, .y = 0 };
	}

	const CGSize fb_size = graphics->metal_layer.drawableSize;
	return (spoopy_vec2_int_t) { .x = (int)fb_size.width, .y = (int)fb_size.height };
}

void* spoopy_graphics_get_native_drawable(spoopy_graphics_t* graphics) {
	return (__bridge void*)graphics->active_drawable;
}

spoopy_graphics_t* spoopy_graphics_new(spoopy_renderer_t renderer) {
	@autoreleasepool {
		spoopy_graphics_t* graphics = spoopy_heap_alloc(sizeof(spoopy_graphics_t));
		graphics->child = (spoopy_graphics_child_t){ 0 };
		graphics->child.renderer = spoopy_graphics_pick_renderer(renderer);
		return graphics;
	}
}


// TODO (WebGPU): Make `renderer` a parameter instead of a constant variable

bool spoopy_graphics_set_mode(spoopy_graphics_t* graphics, void* context_view) {
	switch(graphics->child.renderer) {
		default:
		case SPOOPY_RENDERER_API_METAL:
			return _spoopy_mtl_set_mode(graphics, context_view);
	}

	SPOOPY_LOG_ERROR("Renderer API not supported on this device!");
	return false;
}

spoopy_vec2_int_t spoopy_graphics_update_present(spoopy_graphics_t* graphics) {
	switch(graphics->child.renderer) {
		default:
		case SPOOPY_RENDERER_API_METAL:
			return _spoopy_mtl_update_present(graphics);
	}

	SPOOPY_LOG_ERROR("Renderer API not supported on this device!");
	return (spoopy_vec2_int_t) { .x = 0, .y = 0 };
}
