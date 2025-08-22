#include <spoopy_pipeline.h>
#include <spoopy_shader.h>
#include <spoopy_vertex_attr.h>
#include <memory/spoopy_memory.h>

#include "kore2.h"

static kinc_g4_vertex_data_t vertex_format(
	spoopy_vertex_attr_type_t type,
	spoopy_vertex_attr_conv_t conv,
	uint32_t elements
) {
	static struct {
		uint8_t elements;
		uint8_t type;
		uint8_t conversion;
	} formats[] = {
		[KINC_G4_VERTEX_DATA_F32_1X]  = { 1, SPOOPY_VA_FLOAT, SPOOPY_VA_CONV_FLOAT },
		[KINC_G4_VERTEX_DATA_F32_2X]  = { 2, SPOOPY_VA_FLOAT, SPOOPY_VA_CONV_FLOAT },
		[KINC_G4_VERTEX_DATA_F32_3X]  = { 3, SPOOPY_VA_FLOAT, SPOOPY_VA_CONV_FLOAT },
		[KINC_G4_VERTEX_DATA_F32_4X]  = { 4, SPOOPY_VA_FLOAT, SPOOPY_VA_CONV_FLOAT },

		[KINC_G4_VERTEX_DATA_I8_1X]   = { 1, SPOOPY_VA_BYTE, SPOOPY_VA_CONV_INT },
		[KINC_G4_VERTEX_DATA_I8_2X]   = { 2, SPOOPY_VA_BYTE, SPOOPY_VA_CONV_INT },
		[KINC_G4_VERTEX_DATA_I8_4X]   = { 4, SPOOPY_VA_BYTE, SPOOPY_VA_CONV_INT },

		[KINC_G4_VERTEX_DATA_U8_1X]   = { 1, SPOOPY_VA_UBYTE, SPOOPY_VA_CONV_INT },
		[KINC_G4_VERTEX_DATA_U8_2X]   = { 2, SPOOPY_VA_UBYTE, SPOOPY_VA_CONV_INT },
		[KINC_G4_VERTEX_DATA_U8_4X]   = { 4, SPOOPY_VA_UBYTE, SPOOPY_VA_CONV_INT },

		[KINC_G4_VERTEX_DATA_I8_1X_NORMALIZED] = { 1, SPOOPY_VA_BYTE, SPOOPY_VA_CONV_FLOAT_NORMALIZED },
		[KINC_G4_VERTEX_DATA_I8_2X_NORMALIZED] = { 2, SPOOPY_VA_BYTE, SPOOPY_VA_CONV_FLOAT_NORMALIZED },
		[KINC_G4_VERTEX_DATA_I8_4X_NORMALIZED] = { 4, SPOOPY_VA_BYTE, SPOOPY_VA_CONV_FLOAT_NORMALIZED },

		[KINC_G4_VERTEX_DATA_U8_1X_NORMALIZED] = { 1, SPOOPY_VA_UBYTE, SPOOPY_VA_CONV_FLOAT_NORMALIZED },
		[KINC_G4_VERTEX_DATA_U8_2X_NORMALIZED] = { 2, SPOOPY_VA_UBYTE, SPOOPY_VA_CONV_FLOAT_NORMALIZED },
		[KINC_G4_VERTEX_DATA_U8_4X_NORMALIZED] = { 4, SPOOPY_VA_UBYTE, SPOOPY_VA_CONV_FLOAT_NORMALIZED },

		[KINC_G4_VERTEX_DATA_I16_1X]  = { 1, SPOOPY_VA_SHORT, SPOOPY_VA_CONV_INT },
		[KINC_G4_VERTEX_DATA_I16_2X]  = { 2, SPOOPY_VA_SHORT, SPOOPY_VA_CONV_INT },
		[KINC_G4_VERTEX_DATA_I16_4X]  = { 4, SPOOPY_VA_SHORT, SPOOPY_VA_CONV_INT },

		[KINC_G4_VERTEX_DATA_U16_1X]  = { 1, SPOOPY_VA_USHORT, SPOOPY_VA_CONV_INT },
		[KINC_G4_VERTEX_DATA_U16_2X]  = { 2, SPOOPY_VA_USHORT, SPOOPY_VA_CONV_INT },
		[KINC_G4_VERTEX_DATA_U16_4X]  = { 4, SPOOPY_VA_USHORT, SPOOPY_VA_CONV_INT },

		[KINC_G4_VERTEX_DATA_I16_1X_NORMALIZED] = { 1, SPOOPY_VA_SHORT, SPOOPY_VA_CONV_FLOAT_NORMALIZED },
		[KINC_G4_VERTEX_DATA_I16_2X_NORMALIZED] = { 2, SPOOPY_VA_SHORT, SPOOPY_VA_CONV_FLOAT_NORMALIZED },
		[KINC_G4_VERTEX_DATA_I16_4X_NORMALIZED] = { 4, SPOOPY_VA_SHORT, SPOOPY_VA_CONV_FLOAT_NORMALIZED },

		[KINC_G4_VERTEX_DATA_U16_1X_NORMALIZED] = { 1, SPOOPY_VA_USHORT, SPOOPY_VA_CONV_FLOAT_NORMALIZED },
		[KINC_G4_VERTEX_DATA_U16_2X_NORMALIZED] = { 2, SPOOPY_VA_USHORT, SPOOPY_VA_CONV_FLOAT_NORMALIZED },
		[KINC_G4_VERTEX_DATA_U16_4X_NORMALIZED] = { 4, SPOOPY_VA_USHORT, SPOOPY_VA_CONV_FLOAT_NORMALIZED },

		[KINC_G4_VERTEX_DATA_I32_1X]  = { 1, SPOOPY_VA_INT, SPOOPY_VA_CONV_INT },
		[KINC_G4_VERTEX_DATA_I32_2X]  = { 2, SPOOPY_VA_INT, SPOOPY_VA_CONV_INT },
		[KINC_G4_VERTEX_DATA_I32_3X]  = { 3, SPOOPY_VA_INT, SPOOPY_VA_CONV_INT },
		[KINC_G4_VERTEX_DATA_I32_4X]  = { 4, SPOOPY_VA_INT, SPOOPY_VA_CONV_INT },

		[KINC_G4_VERTEX_DATA_U32_1X]  = { 1, SPOOPY_VA_UINT, SPOOPY_VA_CONV_INT },
		[KINC_G4_VERTEX_DATA_U32_2X]  = { 2, SPOOPY_VA_UINT, SPOOPY_VA_CONV_INT },
		[KINC_G4_VERTEX_DATA_U32_3X]  = { 3, SPOOPY_VA_UINT, SPOOPY_VA_CONV_INT },
		[KINC_G4_VERTEX_DATA_U32_4X]  = { 4, SPOOPY_VA_UINT, SPOOPY_VA_CONV_INT },
	};

	const size_t vtx_array_size = sizeof(formats) / sizeof(formats[0]);
	for(kinc_g4_vertex_data_t vtx=0; vtx<vtx_array_size; ++vtx) {
		if(
			formats[vtx].type == type &&
			formats[vtx].conversion == conv &&
			formats[vtx].elements == elements
		) {
			return vtx;
		}
	}

	SPOOPY_LOG_ERROR("Invalid vertex format: type=%u, conversion=%u, elements=%u", type, conv, elements);
	return KINC_G4_VERTEX_DATA_NONE;
}

struct spoopy_pipeline* spoopy_kinc_pipeline_link(uint32_t num_objs, spoopy_shader_object_t* objs[], uint32_t num_structs) {
    struct spoopy_pipeline* pipeline = SPOOPY_FLEX_ALLOC(struct spoopy_pipeline, sizeof(kinc_g5_vertex_structure_t) * num_structs, spoopy_heap);
    kinc_g5_pipeline_init(&pipeline->core);
    pipeline->core.vertexShader = &objs[0]->core;
    pipeline->core.fragmentShader = &objs[1]->core;

    assert(num_structs < 16); // Maximum number of vertex structures is 16
    for(uint32_t i=0; i<num_structs; i++) {
	    kinc_g5_vertex_structure_init(&pipeline->structures[i]);
        pipeline->core.inputLayout[i] = &pipeline->structures[i];
    }

    return pipeline;
}

void spoopy_kinc_pipeline_compile(struct spoopy_pipeline* pipeline, uint32_t spec_count, spoopy_vertex_attr_spec_t spec[spec_count], uint32_t structure) {
    for(uint32_t i=0; i<spec_count; i++) {
        kinc_g5_vertex_structure_add(
            &pipeline->structures[structure],
            spec[i].name,
			vertex_format(spec[i].type, spec[i].conversion, spec[i].elements)
        );
    }

    pipeline->core.inputLayout[structure] = &pipeline->structures[structure];
	kinc_g5_pipeline_compile(&pipeline->core);
}
