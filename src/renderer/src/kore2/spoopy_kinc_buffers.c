#include <spoopy_pipeline.h>
#include <memory/spoopy_memory.h>
#include <spoopy_types.h>
#include <string.h>

#include "kore2.h"

// TODO (Optimize Memory) - This is a temporary function until we have a proper
// custom vertex buffer implementation ment for sync based optimization for both
// performance and memory usage.
spoopy_vertex_buffer_t* spoopy_kinc_vertex_buffer_create(uint32_t capacity, uint32_t count, void* data, uint32_t structure, spoopy_pipeline_t* pipeline) {
	spoopy_vertex_buffer_t* vbuf = (spoopy_vertex_buffer_t*)spoopy_heap_alloc(sizeof(spoopy_vertex_buffer_t));

	*vbuf = (spoopy_vertex_buffer_t){
		.raw = {0}
	};

	kinc_g4_vertex_structure_t kinc_structure = pipeline->structures[structure];
	kinc_g4_vertex_buffer_init(&vbuf->raw, count, &kinc_structure, KINC_G4_USAGE_STATIC, 0);
	float* v = kinc_g4_vertex_buffer_lock_all(&vbuf->raw);

	if(data) {
		memcpy(v, data, capacity);
	}

	kinc_g4_vertex_buffer_unlock_all(&vbuf->raw);
	return vbuf;
}

spoopy_index_buffer_t* spoopy_kinc_index_buffer_create(uint32_t count, void* data) {
	spoopy_index_buffer_t* ibuf = (spoopy_index_buffer_t*)spoopy_heap_alloc(sizeof(spoopy_index_buffer_t));

	*ibuf = (spoopy_index_buffer_t){
		.raw = {0}
	};

	kinc_g4_index_buffer_init(&ibuf->raw, count, KINC_G4_INDEX_BUFFER_FORMAT_16BIT, KINC_G4_USAGE_STATIC);
	uint16_t* i = kinc_g4_index_buffer_lock_all(&ibuf->raw);

	if(data) {
		memcpy(i, data, count * sizeof(uint16_t));
	}

	kinc_g4_index_buffer_unlock_all(&ibuf->raw);

	return ibuf;
}
