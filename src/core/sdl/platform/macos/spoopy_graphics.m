#include <spoopy_log.h>

#include "../spoopy_graphics.h"

#import <QuartzCore/CAMetalLayer.h>
#import <Metal/Metal.h>

static struct {
	id<MTLDevice> device;
} spoopy_graphics = { 0 };

// TODO (Base Optimize): Support low power mode
void spoopy_graphics_init(void) {
	@autoreleasepool {
		if(@available(macOS 10.15, iOS 13.0, *)) {
			if(spoopy_graphics.device == nil) {
				spoopy_graphics.device = MTLCreateSystemDefaultDevice();
			}

			SPOOPY_LOG_THROW("Metal is not supported on this system!");
		}else {
			SPOOPY_LOG_THROW("Spoopy's Metal graphics backend requires macOS 10.15+ or iOS 13+!");
		}
	}
}
