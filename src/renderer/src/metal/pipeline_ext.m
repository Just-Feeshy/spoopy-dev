#define SPOOPY_IMPL(x) kore_gpu_##x
#include <kore3/gpu/device.h>
#include <spoopy_backend.h>
#include <spoopy_log.h>
#import <Foundation/Foundation.h>
#import <Metal/Metal.h>

void spoopy_metal_pipeline_compile(kore_gpu_device* device, spoopy_shader_source_t* info) {
	NSError* __autoreleasing error = nil;

	switch(info->lang.target) {
		case SLANG_METAL:
			if(!info->context) {
				SPOOPY_LOG_ERROR("No shader source provided for compilation.");
				SPOOPY_LOG_ERROR("No MSL source available");
				return;
			}

			@autoreleasepool {
				NSString* source = [NSString stringWithUTF8String:info->context];
				MTLCompileOptions* options = [[MTLCompileOptions alloc] init];
				if (@available(macOS 13.0, *)) {
					options.languageVersion = MTLLanguageVersion3_0;
				} else {
					options.languageVersion = MTLLanguageVersion2_4;
				}

				id<MTLDevice> metal_device = (__bridge id<MTLDevice>)device->metal.device;
				device->metal.library = (__bridge_retained void *)[metal_device newLibraryWithSource:source options:options error:&error];
			}

			break;

		case SLANG_METAL_LIB:
		case SLANG_METAL_LIB_ASM:
			if(!info->context) {
				SPOOPY_LOG_ERROR("No shader source provided for compilation.");
				SPOOPY_LOG_ERROR("No binary metallib data available");
				return;
			}

			@autoreleasepool {
				NSData* data = [NSData dataWithBytes:info->context length:info->context_size];
				id<MTLDevice> metal_device = (__bridge id<MTLDevice>)device->metal.device;
				id<MTLLibrary> library = [metal_device newLibraryWithData:(dispatch_data_t)data error:&error];
				device->metal.library = (__bridge_retained void *)library;
			}

			break;

		default:
			SPOOPY_LOG_ERROR("Unsupported shader language type: %d", info->lang.target);
			break;
	}
}

pipeline_renderer_funcs_t _pipeline_funcs = {
	.compile = spoopy_metal_pipeline_compile
};
