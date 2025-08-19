#include "../spoopy_kinc_shader.h"
#include <spoopy_log.h>
#include <spoopy_backend.h>
#include <spoopy_shader.h>
#include <kinc/graphics5/graphics.h>
#include <kinc/graphics5/shader.h>
#include <Metal/Metal.h>

id getMetalDevice(void);

void spoopy_kinc_shader_init(spoopy_shader_object_t* shader, spoopy_shader_source_t* info) {
	switch(info->lang.target) {
		case SLANG_METAL_LIB:
		case SLANG_METAL_LIB_ASM:
			break;
		default:
			// TODO (Textures): Add support for SLANG_METAL
			SPOOPY_LOG_WARN("Unsupported shader language target: %d, use [kinc_g5_shader_int] instead!", info->lang.target);
			return;
	}

	strcpy(shader->core.impl.name, info->entry_point);
	id<MTLDevice> device = getMetalDevice();
	NSData* lib_data = [NSData dataWithBytes:info->context
		length:info->context_size];

	NSError* error = nil;
	id<MTLLibrary> library = [device newLibraryWithData:lib_data error:&error];

	if(library == nil) {
		SPOOPY_LOG_ERROR("Failed to create Metal library: %s", error.localizedDescription.UTF8String);
		return;
	}

	shader->core.impl.mtlFunction = (__bridge_retained void *)[library newFunctionWithName:
        [NSString stringWithCString:info->entry_point encoding:NSUTF8StringEncoding]];
}
