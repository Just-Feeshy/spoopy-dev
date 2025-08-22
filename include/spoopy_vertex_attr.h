#pragma once

#ifdef __cplusplus
extern "C" {
#endif

typedef enum spoopy_vertex_attr_conv {
	SPOOPY_VA_CONV_FLOAT,
	SPOOPY_VA_CONV_FLOAT_NORMALIZED,
	SPOOPY_VA_CONV_INT,
} spoopy_vertex_attr_conv_t;

typedef enum spoopy_vertex_attr_type {
	SPOOPY_VA_FLOAT,
	SPOOPY_VA_BYTE,
	SPOOPY_VA_UBYTE,
	SPOOPY_VA_SHORT,
	SPOOPY_VA_USHORT,
	SPOOPY_VA_INT,
	SPOOPY_VA_UINT,
} spoopy_vertex_attr_type_t;

typedef struct spoopy_vertex_attr_spec {
	const char* name;
	uint8_t elements;
	spoopy_vertex_attr_type_t type;
	spoopy_vertex_attr_conv_t conversion;
} spoopy_vertex_attr_spec_t;

#ifdef __cplusplus
}
#endif
