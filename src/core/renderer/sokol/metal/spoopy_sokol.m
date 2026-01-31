#include <spoopy_backend.h>
#include <renderer/spoopy_metal.h>

#include "../spoopy_sokol.h"

void spoopy_sokol_init(void) {
	sg_setup(&(sg_desc) {
		SPOOPY_SG_DEFAULT_DESC

		.environment.metal.device = (__bridge const void*) spoopy_metal_init(),
	});
}
