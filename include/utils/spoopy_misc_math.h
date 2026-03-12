#pragma once

#include <spoopy.h>

#define SPOOPY_PI_F 3.1415926535f
#define SPOOPY_DEG2RAD_F (SPOOPY_PI_F / 180.0f)
#define SPOOPY_RAD2DEG_F (180.0f / SPOOPY_PI_F)

#define spoopy_min(a, b) ({ \
	typeof((a)+(b)) _temp_a = (a); \
	typeof((a)+(b)) _temp_b = (b); \
	(_temp_a > _temp_b) ? _temp_b : _temp_a; \
})

#define spoopy_max(a, b) ({ \
	typeof((a)+(b)) _temp_a = (a); \
	typeof((a)+(b)) _temp_b = (b); \
	(_temp_a > _temp_b) ? _temp_a : _temp_b; \
})

#define spoopy_clamp(x, a, b) spoopy_min(spoopy_max(x, a), b)


SPOOPY_FUNC_CORE size_t spoopy_ceil_pow2_size(size_t x) SPOOPY_ATTR_CONST;
SPOOPY_FUNC_CORE uint32_t spoopy_ceil_pow2_u32(uint32_t v) SPOOPY_ATTR_CONST;
SPOOPY_FUNC_CORE float spoopy_lerp_f32(float a, float b, float t) SPOOPY_ATTR_CONST;
