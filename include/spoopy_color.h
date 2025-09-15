#pragma once

#include <utils/spoopy_compat.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef union spoopy_color3 {
	struct { float r, g, b; };
	float rgb[3];
} spoopy_color3_t;

// TODO (Old System Support) - Test if this works correctly on big-endian systems
typedef union spoopy_color {
	struct { float r, g, b, a; };
	spoopy_color3_t rgb;
	float rgba[4];
	uint32_t packed; // RGBA8 packed as 0xAABBGGRR
} spoopy_color_t;

#define SPOOPY_RGBA(r, g, b, a) (&(spoopy_color_t){ { (r), (g), (b), (a) } })
#define SPOOPY_RGB(r, g, b) SPOOPY_RGBA((r), (g), (b), 1)

#ifdef __cplusplus
}
#endif
