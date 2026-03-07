#include <renderer/spoopy_metal.h>

static id<MTLDevice> g_metal_device = nil;

id<MTLDevice> spoopy_metal_init(void) {
	@autoreleasepool {
		if (g_metal_device != nil) {
			return g_metal_device;
		}

		if (@available(macOS 10.15, iOS 13.0, *)) {
			g_metal_device = MTLCreateSystemDefaultDevice();

			if (g_metal_device == nil) {
				SPOOPY_LOG_THROW("Metal is not supported on this system!");
				return NULL;
			}
		} else {
			SPOOPY_LOG_THROW("Spoopy's Metal graphics backend requires macOS 10.15+ or iOS 13+!");
			return NULL;
		}

		return g_metal_device;
	}
}
