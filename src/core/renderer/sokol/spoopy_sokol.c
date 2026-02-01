#include <spoopy_backend.h>
#include <utils/assert.h>

#define SPOOPY_SOKOL_IMPLEMENTATION
#include "spoopy_sokol.h"

const size_t spoopy_shader_object_size = sizeof(spoopy_shader_object_t);

static_assert(sizeof(spoopy_color_t) >= sizeof(sg_color), "spoopy_color_t must hold sg_color");
static_assert(__alignof(spoopy_color_t) >= __alignof(sg_color), "spoopy_color_t alignment must satisfy sg_color");

// Valid
sg_swapchain spoopy_swapchain = {
	.width = 0,
	.height = 0,
	.sample_count = 1,
	.color_format = SG_PIXELFORMAT_BGRA8,
	.depth_format = SG_PIXELFORMAT_NONE,
};

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


static inline void spoopy_sokol_update_swapchain(spoopy_graphics_t *graphics) {
	spoopy_vec2_int_t fb_size = spoopy_graphics_update_present(graphics);

	switch(spoopy_graphics_get_renderer(graphics)) {
		default:
		case SPOOPY_RENDERER_API_METAL:
			#if defined(SPOOPY_RENDERER_METAL)
			spoopy_swapchain.metal.current_drawable = spoopy_graphics_get_native_drawable(graphics);
			#endif

			break;
	}

	spoopy_swapchain.width  = fb_size.w;
	spoopy_swapchain.height = fb_size.h;
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
				SPOOPY_LOG_INFO("Vertex shader entry: '%s', source len: %zu",
					obj->func.entry ? obj->func.entry : "(null)",
					obj->func.source ? strlen(obj->func.source) : 0);
				break;
			case SPOOPY_STAGE_FRAGMENT:
				desc.fragment_func = obj->func;
				has_fragment = true;
				SPOOPY_LOG_INFO("Fragment shader entry: '%s', source len: %zu",
					obj->func.entry ? obj->func.entry : "(null)",
					obj->func.source ? strlen(obj->func.source) : 0);
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

	SPOOPY_LOG_INFO("Created shader with id=%u", pipeline->shader.id);

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

static void spoopy_sokol_clear(spoopy_graphics_t* graphics, spoopy_buffer_kind_t flags, const spoopy_color_t *color_val, float depth_val) {
	static const sg_load_action load_actions[2] = { SG_LOADACTION_LOAD, SG_LOADACTION_CLEAR };

	sg_pass_action action = {0};
	const uint32_t color_clear = (flags & SPOOPY_BUFFER_COLOR) != 0;
	action.colors[0].load_action = load_actions[color_clear];
	action.colors[0].clear_value = *(const sg_color*)color_val->rgba;

	const uint32_t depth_clear = (flags & SPOOPY_BUFFER_DEPTH) != 0;
	action.depth.load_action = load_actions[depth_clear];
	action.depth.clear_value = depth_val;

	spoopy_swapchain.depth_format = (flags & SPOOPY_BUFFER_DEPTH)
		? SG_PIXELFORMAT_DEPTH_STENCIL
		: SG_PIXELFORMAT_NONE;

	spoopy_sokol_update_swapchain(graphics);

	sg_begin_pass(&(sg_pass) {
		.action = action,
		.swapchain = spoopy_swapchain,
	});
}

// TODO (Windows): Bring back `vertex_shader` parameter to use for attribute semi-name for D3D11
static void spoopy_sokol_pipeline_compile(spoopy_pipeline_t* pipeline, uint32_t spec_count, spoopy_vertex_attr_spec_t spec[spec_count], uint32_t buffer_index) {
	sg_pipeline_desc pdesc = {0};
	pdesc.shader = pipeline->shader;
	pdesc.index_type = SG_INDEXTYPE_UINT16;
	pdesc.color_count = 1;
	pdesc.colors[0].pixel_format = spoopy_swapchain.color_format;
	pdesc.depth.pixel_format = spoopy_swapchain.depth_format;

	// Simplified to match minimal working test - only set what's necessary
	for(uint32_t i = 0; i < spec_count && i < SG_MAX_VERTEX_ATTRIBUTES; i++) {
		pdesc.layout.attrs[i].format = spoopy_sokol_vertex_format(&spec[i]);
	}

	pipeline->pipeline = sg_make_pipeline(&pdesc);
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

static size_t spoopy_sokol_buffer_size(spoopy_buffer_type_t type) {
	switch (type) {
		case SPOOPY_BUFFER_TYPE_VERTEX:
			return sizeof(spoopy_vertex_buffer_t);
		case SPOOPY_BUFFER_TYPE_INDEX:
			return sizeof(spoopy_index_buffer_t);
		default:
			SPOOPY_LOG_ERROR("Unknown buffer type: %d", type);
			return 0;
	}
}

static bool spoopy_sokol_vertex_buffer_create(spoopy_vertex_buffer_t* buffer, uint32_t capacity, uint32_t count, void* data, uint32_t stride) {
	if(SPOOPY_UNLIKELY(!buffer)) {
		return false;
	}

	memset(buffer, 0, sizeof(*buffer));
	buffer->stride = stride;
	buffer->count = count;

	if(!buffer) {
		SPOOPY_LOG_ERROR("Vertex buffer output pointer is NULL");
		return false;
	}

	sg_buffer_desc desc = {0};
	desc.usage.vertex_buffer = true; // Valid
	desc.usage.immutable = true; // Valid

	if(data) { // Valud
		desc.data.ptr = data;
		desc.data.size = capacity;
	} else {
		desc.size = capacity;
	}

	sg_buffer buf = sg_make_buffer(&desc);
	if(buf.id == SG_INVALID_ID) {
		SPOOPY_LOG_ERROR("Failed to create Sokol vertex buffer");
		return false;
	}

	buffer->buffer = buf;
	return true;
}

static bool spoopy_sokol_index_buffer_create(spoopy_index_buffer_t* buffer, uint32_t count, void* data) {
	if(SPOOPY_UNLIKELY(!buffer)) {
		return false;
	}

	memset(buffer, 0, sizeof(*buffer));
	buffer->count = count;

	const size_t buffer_size = (size_t)count * sizeof(uint16_t);
	if(buffer_size == 0) {
		SPOOPY_LOG_ERROR("Index buffer size must be greater than 0");
		return false;
	}

	sg_buffer_desc desc = {0};
	desc.usage.index_buffer = true;
	desc.usage.immutable = true; // Valid
	if(data) { // Valid
		desc.data.ptr = data;
		desc.data.size = buffer_size;
	} else {
		desc.size = buffer_size;
	}

	sg_buffer buf = sg_make_buffer(&desc);
	if(buf.id == SG_INVALID_ID) {
		SPOOPY_LOG_ERROR("Failed to create Sokol index buffer");
		return false;
	}

	buffer->buffer = buf;
	return true;
}

static void spoopy_sokol_pipeline_bind(spoopy_pipeline_t* pipeline) {
	sg_apply_pipeline(pipeline->pipeline);
}

static void spoopy_sokol_draw_mesh(const spoopy_mesh_t* mesh, spoopy_pipeline_t* pipeline) {
	pipeline->bindings = (sg_bindings){ 0 };

	const uint16_t max_buffers = mesh->vertex_count;
	assert(max_buffers <= SG_MAX_VERTEXBUFFER_BINDSLOTS);

	for(uint16_t i=0; i<max_buffers; i++) {
		pipeline->bindings.vertex_buffers[i] = mesh->vertex_buffers[i].buffer;
	}

	if(mesh->index_buffer) {
		pipeline->bindings.index_buffer = mesh->index_buffer->buffer;
	}

	sg_apply_bindings(&pipeline->bindings);

	if(mesh->index_buffer && mesh->index_count != 0) {
		sg_draw(0, mesh->index_count, 1);
		return;
	}

	if(mesh->vertex_count != 0) {
		sg_draw(0, mesh->vertex_buffers[0].count, 1);
	}
}

static void spoopy_sokol_swap_buffers(void) {
	sg_end_pass();
    sg_commit();
}

static void spoopy_sokol_shutdown(void) {
	sg_shutdown();
}

spoopy_backend_funcs_t _backend_funcs = {
	.init = spoopy_sokol_init,
	.shutdown = spoopy_sokol_shutdown,
	.shader_init = spoopy_sokol_shader_init,
	.shader_destroy = spoopy_sokol_shader_destroy,
	.pipeline_link = spoopy_sokol_pipeline_link,
	.pipeline_compile = spoopy_sokol_pipeline_compile,
	.vertex_buffer_create = spoopy_sokol_vertex_buffer_create,
	.index_buffer_create = spoopy_sokol_index_buffer_create,
	.buffer_size = spoopy_sokol_buffer_size,
	.clear = spoopy_sokol_clear,
	.pipeline_bind = spoopy_sokol_pipeline_bind,
	.draw_mesh = spoopy_sokol_draw_mesh,
	.swap_buffers = spoopy_sokol_swap_buffers,
};
