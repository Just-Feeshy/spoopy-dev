#pragma once

#ifdef __cplusplus
extern "C" {
#endif

typedef enum spoopy_buffer_kind {
	SPOOPY_BUFFER_COLOR = (1 << 0),
	SPOOPY_BUFFER_DEPTH = (1 << 1),

	SPOOPY_BUFFER_ALL = SPOOPY_BUFFER_COLOR | SPOOPY_BUFFER_DEPTH
} spoopy_buffer_kind_t;

typedef struct spoopy_vertex_buffer spoopy_vertex_buffer_t;
typedef struct spoopy_vertex_array spoopy_vertex_array_t;
typedef struct spoopy_index_buffer spoopy_index_buffer_t;
typedef struct spoopy_mesh spoopy_mesh_t;

struct spoopy_mesh {
	spoopy_vertex_buffer_t* vertex_buffer;
	spoopy_index_buffer_t* index_buffer;
	uint32_t index_count;
};

#ifdef __cplusplus
}
#endif
