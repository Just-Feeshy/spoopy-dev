#include <spoopy_graphics.h>

static bool __gamma_correction = false;

void spoopy_graphics_set_gamma_correction(bool gamma_correction) {
	__gamma_correction = gamma_correction;
}

bool spoopy_graphics_get_gamma_correction(void) {
	return __gamma_correction;
}
