#include <spoopy_backend.h>

#include "spoopy_sokol.h"

static const struct {
	uint8_t elements;
	uint8_t type;
	uint8_t conversion;
} formats[] = {
	[SG_VERTEXFORMAT_FLOAT]  = { 1, SPOOPY_VA_FLOAT, SPOOPY_VA_CONV_FLOAT },
	[SG_VERTEXFORMAT_FLOAT2] = { 2, SPOOPY_VA_FLOAT, SPOOPY_VA_CONV_FLOAT },
	[SG_VERTEXFORMAT_FLOAT3] = { 3, SPOOPY_VA_FLOAT, SPOOPY_VA_CONV_FLOAT },
	[SG_VERTEXFORMAT_FLOAT4] = { 4, SPOOPY_VA_FLOAT, SPOOPY_VA_CONV_FLOAT },

	[SG_VERTEXFORMAT_INT]  = { 1, SPOOPY_VA_INT, SPOOPY_VA_CONV_INT },
	[SG_VERTEXFORMAT_INT2] = { 2, SPOOPY_VA_INT, SPOOPY_VA_CONV_INT },
	[SG_VERTEXFORMAT_INT3] = { 3, SPOOPY_VA_INT, SPOOPY_VA_CONV_INT },
	[SG_VERTEXFORMAT_INT4] = { 4, SPOOPY_VA_INT, SPOOPY_VA_CONV_INT },

	[SG_VERTEXFORMAT_UINT]  = { 1, SPOOPY_VA_UINT, SPOOPY_VA_CONV_INT },
	[SG_VERTEXFORMAT_UINT2] = { 2, SPOOPY_VA_UINT, SPOOPY_VA_CONV_INT },
	[SG_VERTEXFORMAT_UINT3] = { 3, SPOOPY_VA_UINT, SPOOPY_VA_CONV_INT },
	[SG_VERTEXFORMAT_UINT4] = { 4, SPOOPY_VA_UINT, SPOOPY_VA_CONV_INT },

	[SG_VERTEXFORMAT_BYTE4]  = { 4, SPOOPY_VA_BYTE, SPOOPY_VA_CONV_INT },
	[SG_VERTEXFORMAT_BYTE4N] = { 4, SPOOPY_VA_BYTE, SPOOPY_VA_CONV_FLOAT_NORMALIZED },

	[SG_VERTEXFORMAT_UBYTE4]  = { 4, SPOOPY_VA_UBYTE, SPOOPY_VA_CONV_INT },
	[SG_VERTEXFORMAT_UBYTE4N] = { 4, SPOOPY_VA_UBYTE, SPOOPY_VA_CONV_FLOAT_NORMALIZED },

	[SG_VERTEXFORMAT_SHORT2]  = { 2, SPOOPY_VA_SHORT, SPOOPY_VA_CONV_INT },
	[SG_VERTEXFORMAT_SHORT2N] = { 2, SPOOPY_VA_SHORT, SPOOPY_VA_CONV_FLOAT_NORMALIZED },
	[SG_VERTEXFORMAT_SHORT4]  = { 4, SPOOPY_VA_SHORT, SPOOPY_VA_CONV_INT },
	[SG_VERTEXFORMAT_SHORT4N] = { 4, SPOOPY_VA_SHORT, SPOOPY_VA_CONV_FLOAT_NORMALIZED },

	[SG_VERTEXFORMAT_USHORT2]  = { 2, SPOOPY_VA_USHORT, SPOOPY_VA_CONV_INT },
	[SG_VERTEXFORMAT_USHORT2N] = { 2, SPOOPY_VA_USHORT, SPOOPY_VA_CONV_FLOAT_NORMALIZED },
	[SG_VERTEXFORMAT_USHORT4]  = { 4, SPOOPY_VA_USHORT, SPOOPY_VA_CONV_INT },
	[SG_VERTEXFORMAT_USHORT4N] = { 4, SPOOPY_VA_USHORT, SPOOPY_VA_CONV_FLOAT_NORMALIZED },
};

static const size_t vtx_array_size = sizeof(formats) / sizeof(formats[0]);


static const char* spoopy_sokol_d3d_target(spoopy_shader_stage_t stage) {
	switch (stage) {
		case SPOOPY_STAGE_VERTEX:
			return "vs_5_0";
		case SPOOPY_STAGE_FRAGMENT:
			return "ps_5_0";
		default:
			return NULL;
	}
}

static void spoopy_sokol_shader_init(spoopy_shader_object_t* shader, spoopy_shader_source_t* info) {
	if(!shader || !info) {
		SPOOPY_LOG_ERROR("Invalid shader init params");
		return;
	}

	*shader = (spoopy_shader_object_t){0};
	shader->stage = info->stage;

	if(info->content) {
		size_t src_len = info->content_size > 0 ? info->content_size : strlen(info->content);
		char* copy = spoopy_heap_strndup(info->content, src_len);
		if(!copy) {
			SPOOPY_LOG_ERROR("Out of memory copying shader source");
			return;
		}

		shader->func.source = copy;
		shader->owns_source = true;
	}
	shader->func.entry = info->entry_point;

	const spoopy_renderer_t renderer = spoopy_graphics_pick_renderer(info->target);
	if(!spoopy_graphics_renderer_is_single(renderer)) {
		SPOOPY_LOG_ERROR("Cannot support multiple renderers!");
		return;
	}

	if(renderer & SPOOPY_RENDERER_API_D3D11) {
		shader->func.d3d11_target = spoopy_sokol_d3d_target(info->stage);
	}
}

static spoopy_pipeline_t* spoopy_sokol_pipeline_link(uint32_t num_objs, spoopy_shader_object_t* objs[]) {
	if(num_objs == 0 || !objs) {
		SPOOPY_LOG_ERROR("No shader objects provided for pipeline link");
		return NULL;
	}

	sg_shader_desc desc = {0};
	bool has_vertex = false;
	bool has_fragment = false;

	for(uint32_t i=0; i<num_objs; i++) {
		spoopy_shader_object_t* obj = objs[i];

		if(!obj) {
			continue;
		}

		switch(obj->stage) {
			case SPOOPY_STAGE_VERTEX:
				desc.vertex_func = obj->func;
				has_vertex = true;
				break;
			case SPOOPY_STAGE_FRAGMENT:
				desc.fragment_func = obj->func;
				has_fragment = true;
				break;
			default:
				SPOOPY_LOG_WARN("Unsupported shader stage: %d", obj->stage);
				break;
		}
	}

	if(!has_vertex || !has_fragment) {
		SPOOPY_LOG_ERROR("Pipeline link requires both vertex and fragment shaders");
		return NULL;
	}

	spoopy_pipeline_t* pipeline = spoopy_heap_alloc(sizeof(*pipeline));
	assert(pipeline);

	*pipeline = (spoopy_pipeline_t){0};
	pipeline->shader = sg_make_shader(&desc);

	if(pipeline->shader.id == SG_INVALID_ID) {
		SPOOPY_LOG_ERROR("Failed to create Sokol shader");
		spoopy_heap_free(pipeline);
		return NULL;
	}

	return pipeline;
}

static sg_vertex_format spoopy_sokol_vertex_format(const spoopy_vertex_attr_spec_t* spec) {
	for (sg_vertex_format vtx = 0; vtx < vtx_array_size; ++vtx) {
		if (
			formats[vtx].type == spec->type &&
			formats[vtx].conversion == spec->conversion &&
			formats[vtx].elements == spec->elements
		) {
			return vtx;
		}
	}

	SPOOPY_LOG_ERROR("Invalid vertex format: type=%u, conversion=%u, elements=%u",
		spec->type, spec->conversion, spec->elements);
	return SG_VERTEXFORMAT_INVALID;
}

// TODO (Windows): Brind back the `vertex_shader` parameter to use for attribute semi-name for D3D11
static void spoopy_sokol_pipeline_compile(spoopy_pipeline_t* pipeline, uint32_t spec_count, spoopy_vertex_attr_spec_t spec[spec_count], uint32_t structure) {
	assert(pipline->shader.id != SG_INVALID_ID);

	if(spec_count == 0 || !spec) {
		SPOOPY_LOG_ERROR("No vertex attributes provided");
		return;
	}

	sg_pipeline_desc pdesc = {0};
	pdesc.shader = pipeline->shader;
	pdesc.index_type = SG_INDEXTYPE_UINT16;
	pdesc.layout.buffers[structure].step_func = SG_VERTEXSTEP_PER_VERTEX;

	uint32_t offset = 0;
	const uint32_t attr_count = (spec_count > SG_MAX_VERTEX_ATTRIBUTES) ? SG_MAX_VERTEX_ATTRIBUTES : spec_count;

	for(uint32_t i = 0; i < attr_count; i++) {
		const sg_vertex_format fmt = spoopy_sokol_vertex_format(&spec[i]);
		if(fmt == SG_VERTEXFORMAT_INVALID) {
			SPOOPY_LOG_ERROR("Unsupported vertex attribute format at index %u", i);
			return;
		}

		pdesc.layout.attrs[i].format = fmt;
		pdesc.layout.attrs[i].offset = offset;
		pdesc.layout.attrs[i].buffer_index = 0;
		offset += formats[fmt].elements;
	}

	pdesc.layout.buffers[structure].stride = offset;
	pipeline->pipeline = sg_make_pipeline(&pdesc);
	if(pipeline->pipeline.id == SG_INVALID_ID) {
		SPOOPY_LOG_ERROR("Failed to create Sokol pipeline");
	}
}

static void spoopy_sokol_shader_destroy(spoopy_shader_object_t* shader, bool must_free) {
	if(!shader) {
		SPOOPY_LOG_WARN("`shader` provided was (null)!");
		return;
	}

	if(shader->owns_source && shader->func.source) {
		spoopy_heap_free((void*)shader->func.source);
	}
}

static bool spoopy_sokol_vertex_buffer_create(spoopy_vertex_buffer_t* buffer, uint32_t capacity, uint32_t count, void* data, uint32_t structure, spoopy_pipeline_t* pipeline) {
	(void)pipeline;

	memset(buffer, 0, sizeof(*buffer));
	buffer->stride = (structure != 0) ? structure : 0;
	buffer->count = count;

	const uint32_t size = (uint32_t)capacity;
	if(size == 0) {
		SPOOPY_LOG_ERROR("Vertex buffer size must be > 0");
		return NULL;
	}

	if(!buffer) {
		SPOOPY_LOG_ERROR("Vertex buffer output pointer is NULL");
		return false;
	}

	sg_buffer_desc desc = {0};
	desc.usage.vertex_buffer = true;
	desc.usage.immutable = true;
	if(data) {
		desc.data.ptr = data;
		desc.data.size = size;
	} else {
		desc.size = size;
	}

	sg_buffer buf = sg_make_buffer(&desc);
	if(buf.id == SG_INVALID_ID) {
		SPOOPY_LOG_ERROR("Failed to create Sokol vertex buffer");
		return false;
	}

	buffer->handle = buf.id;
	return true;
}

spoopy_backend_funcs_t _backend_funcs = {
	.init = spoopy_sokol_init,
	.shader_init = spoopy_sokol_shader_init,
	.spoopy_pipeline_link = spoopy_sokol_pipeline_link,
	.pipeline_compile = spoopy_sokol_pipeline_compile,
	.vertex_buffer_create = spoopy_sokol_vertex_buffer_create,
};
