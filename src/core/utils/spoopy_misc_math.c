#include <utils/spoopy_misc_math.h>
#include <spoopy_log.h>

size_t size_t spoopy_ceil_pow2_size(size_t v) {
	if (v <= 1) return 1;

	const size_t bits = sizeof(size_t) * 8;
	const size_t top = (size_t)1 << (bits - 1);
	if (v > top) {
		SPOOPY_LOG_ERROR("spoopy_ceil_pow2_size: input value %zu is too large, returning 0", v);
		return 0;
	}

	v--;
	for (size_t shift = 1; shift < sizeof(size_t) * 8; shift <<= 1) {
		v |= v >> shift;
	}
	v++;

	return v;
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
