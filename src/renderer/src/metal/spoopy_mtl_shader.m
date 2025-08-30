#include <utils/assert.h>
#include <spoopy_log.h>
#include <spoopy.h>
#include <memory/spoopy_memory.h>
#include <spoopy_shader.h>

#include <Metal/Metal.h>

#ifdef KORE_METAL
#include "../kore2/kore2.h"
#endif

#define PARSE_METALLIB(profile, major, minor) \
    ((profile) != NULL && sscanf((profile), "metallib_%hu_%hu", &(major), &(minor)) == 2)

id getMetalDevice(void);

spoopy_shader_object_t* spoopy_mtl_shader_init(spoopy_shader_object_t* shader, spoopy_shader_source_t* info) {
	assert(info != NULL);

	@autoreleasepool {
		id<MTLDevice> device = getMetalDevice();

		uint16_t major, minor;
		if(!PARSE_METALLIB(info->lang.profile, major, minor)) {
			SPOOPY_LOG_ERROR("Invalid Metal shader profile: %s", info->lang.profile);
			return NULL;
		}

		NSError* error = nil;
		MTLCompileOptions* options = [[MTLCompileOptions alloc] init];
		options.languageVersion = (MTLLanguageVersion)(major << 16 | minor);

		id<MTLLibrary> library = [device newLibraryWithSource:[[NSString alloc] initWithBytes:info->content length:info->content_size encoding:NSUTF8StringEncoding] options:options error:&error];
		if(library == nil) {
			SPOOPY_LOG_ERROR("Failed to create Metal shader library: %s\n", error.localizedDescription.UTF8String);
			return NULL;
		}

		id<MTLFunction> function = [library newFunctionWithName:[NSString stringWithUTF8String:info->entry_point]];
		if(function == nil) {
			SPOOPY_LOG_ERROR("Failed to find Metal function: %s", info->entry_point);
			return NULL;
		}

		spoopy_shader_object_t* new_shader;
		if (info->stage == SPOOPY_STAGE_VERTEX) {
			NSUInteger attr_count = function.vertexAttributes.count;
			new_shader = SPOOPY_FLEX_ALLOC(spoopy_shader_object_t, sizeof(const char*) * attr_count, spoopy_heap);
			new_shader->attr_count = (uint32_t)attr_count;

			for (NSUInteger index = 0; index < attr_count; index++) {
				MTLVertexAttribute *attr = function.vertexAttributes[index];
				new_shader->attr_names[index] = spoopy_heap_strdup(attr.name.UTF8String);
			}
		} else {
			new_shader = SPOOPY_FLEX_ALLOC(spoopy_shader_object_t, 0, spoopy_heap);
			new_shader->attr_count = 0;
		}

		new_shader->core.impl.mtlFunction = (__bridge_retained void*)function;
		return new_shader;
	}
}
