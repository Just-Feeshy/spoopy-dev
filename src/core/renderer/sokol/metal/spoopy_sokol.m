#include <spoopy_backend.h>
#include <renderer/spoopy_metal.h>

// Simple any easy unity solution to handle Sokol for Darwin platforms
#include "../spoopy_sokol.inc.h"

void spoopy_sokol_init(void) {
	sg_setup(&(sg_desc) {
		SPOOPY_SG_DEFAULT_DESC

		.environment.metal.device = (__bridge const void*) spoopy_metal_init(),
	});
}
