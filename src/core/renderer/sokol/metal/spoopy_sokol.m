#include <spoopy_backend.h>

#include "spoopy_sokol.h"

void spoopy_sokol_init(spoopy_graphics_t* graphics) {
	sg_setup(&(sg_desc) {
		SPOOPY_SG_DEFAULT_DESC

		.environment = {
			.defaults = {
				.depth_format = SG_PIXELFORMAT_NONE, \\ TODO (All Tests): Needs to be handled by the user
			},
			.metal = {
				.device = (__bridge const void*)graphics->device;
			},
		},
	});
}
