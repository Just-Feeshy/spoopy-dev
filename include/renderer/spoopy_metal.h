#pragma once

#include <spoopy_log.h>

#import <QuartzCore/CAMetalLayer.h>
#import <Metal/Metal.h>

#include <spoopy_log.h>

static struct {
	id<MTLDevice> device;
} spoopy_metal;

//  TODO (Base Optimize): Support low power mode
static inline id<MTLDevice> spoopy_metal_init(void) {
	@autoreleasepool {
		if(@available(macOS 10.15, iOS 13.0, *)) {
			spoopy_metal.device = MTLCreateSystemDefaultDevice();

			if(spoopy_metal.device == nil) {
				SPOOPY_LOG_THROW("Metal is not supported on this system!");
				return NULL;
			}
		}else {
			SPOOPY_LOG_THROW("Spoopy's Metal graphics backend requires macOS 10.15+ or iOS 13+!");
		}

		return spoopy_metal.device;
	}
}
