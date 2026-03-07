#include <utils/spoopy_misc_math.h>
#include <spoopy_log.h>

size_t spoopy_ceil_pow2_size(size_t x) {
	if (x <= 1) return 1;
	x--;
	x |= x >> 1;
	x |= x >> 2;
	x |= x >> 4;
	x |= x >> 8;
	x |= x >> 16;
#if SIZE_MAX > 0xFFFFFFFFu
	x |= x >> 32;
#endif
	return x + 1;
}

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
