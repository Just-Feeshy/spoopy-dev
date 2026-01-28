#define SPOOPY_GRAPHICS_IMPL

#include <SDL3/SDL.h>
#include <SDL3/SDL_metal.h>

#include <spoopy_log.h>
#include <spoopy_graphics.h>
#include <memory/spoopy_memory.h>

#import <QuartzCore/CAMetalLayer.h>
#import <Metal/Metal.h>

static struct {
	id<MTLDevice> device;
} app_device;

struct spoopy_graphics {
	spoopy_graphics_child_t child;

	CAMetalLayer* metal_layer;
	id<CAMetalDrawable> active_drawable;
	id<MTLDevice> device;
};

// TODO (Base Optimize): Support low power mode
void spoopy_graphics_init(void) {
	@autoreleasepool {
		if(@available(macOS 10.15, iOS 13.0, *)) {
			if(app_device.device == nil) {
				app_device.device = MTLCreateSystemDefaultDevice();
				return;
			}

			SPOOPY_LOG_THROW("Metal is not supported on this system!");
		}else {
			SPOOPY_LOG_THROW("Spoopy's Metal graphics backend requires macOS 10.15+ or iOS 13+!");
		}
	}
}

spoopy_graphics_t* spoopy_graphics_new(spoopy_renderer_t renderer) {
	@autoreleasepool {
		spoopy_graphics_t* graphics = spoopy_heap_alloc(sizeof(spoopy_graphics_t));
		graphics->child = (spoopy_graphics_child_t){ 0 };
		graphics->child.renderer = renderer;
		graphics->device = app_device.device;
		return graphics;
	}
}

bool spoopy_graphics_set_mode(spoopy_graphics_t* graphics, void* context_view) {
	@autoreleasepool {
		SDL_MetalView view = (SDL_MetalView)context_view;
		graphics->metal_layer = (CAMetalLayer*)SDL_Metal_GetLayer(view);

		graphics->metal_layer.device = graphics->device;
		graphics->metal_layer.pixelFormat = spoopy_graphics_get_gamma_correction()
			? MTLPixelFormatBGRA8Unorm_sRGB
			: MTLPixelFormatBGRA8Unorm;

		graphics->metal_layer.framebufferOnly = YES;

		return true;
	}
}
