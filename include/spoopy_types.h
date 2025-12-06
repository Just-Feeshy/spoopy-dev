#pragma once

#include <spoopy_image.h>


/** Macro Definitions **/

#define SPOOPY_VEC_AND_REC_TYPES(type) \
	typedef union spoopy_vec2_##type { \
		struct { \
			type x, y; \
		}; \
		struct { \
			type w, h; \
		}; \
		struct { \
			type r, g; \
		}; \
		type data[2]; \
	} spoopy_vec2_##type##_t; \
	typedef union spoopy_vec3_##type { \
		struct { \
			type x, y, z; \
		}; \
		type data[3]; \
	} spoopy_vec3_##type##_t; \
	typedef union spoopy_vec4_##type { \
		struct { \
			type x, y, z, w; \
		}; \
		type data[4]; \
	} spoopy_vec4_##type##_t; \
	typedef struct spoopy_rec_##type { \
		spoopy_vec2_##type##_t point; \
		spoopy_vec2_##type##_t size; \
	} spoopy_rec_##type##_t;


#ifdef __cplusplus
extern "C" {
#endif

/* ============================= Spoopy Core Types =============================
 * All core types used by the Spoopy Environment and Renderer API are
 * defined in this header, it's more or less a nice one stop shop for
 * all the basic structures and types used throughout the literal entire
 * codebase
 * =============================================================================
 */

SPOOPY_VEC_AND_REC_TYPES(float)
SPOOPY_VEC_AND_REC_TYPES(int)


/** Forward Declarations of Core Components **/

typedef struct spoopy_vertex_buffer spoopy_vertex_buffer_t;
typedef struct spoopy_vertex_array spoopy_vertex_array_t;
typedef struct spoopy_index_buffer spoopy_index_buffer_t;
typedef struct spoopy_mesh spoopy_mesh_t;
typedef struct spoopy_preset_vertex_model spoopy_preset_vertex_model_t;
typedef struct spoopy_texture spoopy_texture_t;
typedef struct spoopy_uniform spoopy_uniform_t;
typedef struct spoopy_pipeline spoopy_pipeline_t;
typedef struct spoopy_window* spoopy_window_t;


/** Defined Structures **/

typedef struct spoopy_file_read_callbacks {
	size_t (*read)(void* user_data, void* data, size_t size);
	size_t (*size)(void* user_data);
	size_t (*pos)(void* user_data);
	void   (*seek)(void* user_data, size_t pos);
} spoopy_file_read_callbacks_t;

struct spoopy_mesh {
	spoopy_vertex_buffer_t* vertex_buffer;
	spoopy_index_buffer_t* index_buffer;
	uint32_t index_count;
};

struct spoopy_preset_vertex_model {
	spoopy_vec3_float_t position, normal;
	spoopy_vec2_float_t uv;
	spoopy_vec4_float_t tangent;
};

typedef enum spoopy_buffer_kind {
	SPOOPY_BUFFER_COLOR = (1 << 0),
	SPOOPY_BUFFER_DEPTH = (1 << 1),

	SPOOPY_BUFFER_ALL = SPOOPY_BUFFER_COLOR | SPOOPY_BUFFER_DEPTH
} spoopy_buffer_kind_t;

typedef enum spoopy_texture_filter_mode {
	SPOOPY_TEXTURE_FILTER_LINEAR,
	SPOOPY_TEXTURE_FILTER_LINEAR_MIPMAP_NEAREST,
	SPOOPY_TEXTURE_FILTER_LINEAR_MIPMAP_LINEAR,
	SPOOPY_TEXTURE_FILTER_NEAREST,
	SPOOPY_TEXTURE_FILTER_NEAREST_MIPMAP_NEAREST,
	SPOOPY_TEXTURE_FILTER_NEAREST_MIPMAP_LINEAR,
} spoopy_texture_filter_mode_t;

typedef enum spoopy_texture_wrap_mode {
	SPOOPY_TEXTURE_WRAP_REPEAT,
	SPOOPY_TEXTURE_WRAP_CLAMP,
	SPOOPY_TEXTURE_WRAP_MIRROR,
} spoopy_texture_wrap_mode_t;

typedef enum spoopy_texture_class {
	SPOOPY_TEXTURE_CLASS_2D,
	SPOOPY_TEXTURE_CLASS_CUBE,
} spoopy_texture_class_t;

typedef struct spoopy_texture_params {
	uint32_t width;
	uint32_t height;
	uint32_t layers;
	uint32_t mipmaps;
	spoopy_image_format_t format;
	spoopy_texture_class_t texture_class;

	struct {
		spoopy_texture_filter_mode_t min;
		spoopy_texture_filter_mode_t mag;
	} filter;

	struct {
		spoopy_texture_wrap_mode_t u;
		spoopy_texture_wrap_mode_t v;
	} wrap;
} spoopy_texture_params_t;

typedef enum spoopy_aspect_axis {
	SPOOPY_ASPECT_AXIS_WIDTH,
	SPOOPY_ASPECT_AXIS_HEIGHT,
	SPOOPY_ASPECT_AXIS_NONE,
} spoopy_aspect_axis_t;

typedef enum spoopy_window_flags {
	SPOOPY_WINDOW_FLAG_RESIZABLE = (1 << 0),
	SPOOPY_WINDOW_FLAG_FULLSCREEN = (1 << 1),
	SPOOPY_WINDOW_FLAG_BORDERLESS = (1 << 2),
	SPOOPY_WINDOW_FLAG_HIDDEN = (1 << 3),
	SPOOPY_WINDOW_FLAG_VSYNC = (1 << 4),
} spoopy_window_flags_t;

#ifdef __cplusplus
}
#endif
