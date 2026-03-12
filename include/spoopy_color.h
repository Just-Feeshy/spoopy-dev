#pragma once

#include <utils/spoopy_compat.h>

#ifdef __cplusplus
extern "C" {
#endif

/* TODO (Old System Support) - Test if this works correctly on big-endian systems */


// The reason why I'm not using spoopy_vec types here is to avoid
// any potential padding/alignment issues that may arise from using structs
// with multiple members. Using arrays ensures a consistent memory layout, since we have
// a `packed` member for each color type as well, which is important for interoperability
// and I'm worried it might screw things up on some platforms

typedef union spoopy_color1 {
	float value[1];
	struct { float r; };
	uint8_t packed; // R8 packed as 0xRR
} spoopy_color1_t;

typedef union spoopy_color2 {
	struct { float r, g; };
	float rg[2];
	uint16_t packed; // RG16 packed as 0xGGRR
} spoopy_color2_t;

typedef union spoopy_color3 {
	struct { float r, g, b; };
	float rgb[3];
} spoopy_color3_t;
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
