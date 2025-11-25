#pragma once

#include <spoopy.h>

#define spoopy_min(a, b) ({ \
	typeof((a)+(b)) _temp_a = (a); \
	typeof((a)+(b)) _temp_b = (b); \
	_temp_a > _temp_b ? _temp_b : _temp_a; \
})

#define spoopy_max(a, b) ({ \
	typeof((a)+(b)) _temp_a = (a); \
	typeof((a)+(b)) _temp_b = (b); \
	_temp_a > _temp_b ? _temp_a : _temp_b; \
})

SPOOPY_FUNC_CORE uint32_t spoopy_ceil_pow2_u32(uint32_t v);
