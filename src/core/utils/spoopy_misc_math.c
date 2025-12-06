#include <utils/spoopy_misc_math.h>

uint32_t spoopy_ceil_pow2_u32(uint32_t v) {
	if (v == 0) return 1;

	v--;
	v |= v >> 1;
	v |= v >> 2;
	v |= v >> 4;
	v |= v >> 8;
	v |= v >> 16;
	v++;

	return v;
}

float spoopy_lerp_f32(float a, float b, float t) {
	return a + t * (b - a);
}
