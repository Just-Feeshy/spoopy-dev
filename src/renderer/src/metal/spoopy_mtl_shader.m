#include <utils/assert.h>
#include <spoopy_log.h>
#include <spoopy.h>
#include <spoopy_shader.h>

#include <Metal/Metal.h>

#ifdef KORE_METAL
#include "../kore2/kore2.h"
#endif

#define PARSE_METALLIB(profile, major, minor) \
    ((profile) != NULL && sscanf((profile), "metallib_%hu_%hu", &(major), &(minor)) == 2)

id getMetalDevice(void);

void spoopy_mtl_shader_init(spoopy_shader_object_t* shader, spoopy_shader_source_t* info) {
	assert(shader != NULL && info != NULL);

	@autoreleasepool {
		id<MTLDevice> device = getMetalDevice();

		uint16_t major, minor;
		if(!PARSE_METALLIB(info->lang.profile, major, minor)) {
			SPOOPY_LOG_ERROR("Invalid Metal shader profile: %s", info->lang.profile);
			return;
		}

		NSError* error = nil;
		MTLCompileOptions* options = [[MTLCompileOptions alloc] init];
		options.languageVersion = (MTLLanguageVersion)(major << 16 | minor);

		id<MTLLibrary> library = [device newLibraryWithSource:[[NSString alloc] initWithBytes:info->content length:info->content_size encoding:NSUTF8StringEncoding] options:options error:&error];
		if(library == nil) {
			SPOOPY_LOG_ERROR("Failed to create Metal shader library: %s", error.localizedDescription.UTF8String);
			return;
		}

		id<MTLFunction> function = [library newFunctionWithName:[NSString stringWithUTF8String:info->entry_point]];
		if(function == nil) {
			SPOOPY_LOG_ERROR("Failed to find Metal function: %s", info->entry_point);
			return;
		}

		shader->core.impl.mtlFunction = (__bridge_retained void*)function;
	}
}
