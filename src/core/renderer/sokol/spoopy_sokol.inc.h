#include <spoopy_api.h>
#include <spoopy_backend.h>
#include <spoopy_image.h>
#include <utils/assert.h>

#define SPOOPY_SOKOL_IMPLEMENTATION
#include "spoopy_sokol.h"

const size_t spoopy_shader_object_size = sizeof(spoopy_shader_object_t);

static_assert(sizeof(spoopy_color_t) >= sizeof(sg_color), "spoopy_color_t must hold sg_color");
static_assert(__alignof(spoopy_color_t) >= __alignof(sg_color), "spoopy_color_t alignment must satisfy sg_color");

// TODO (Mutli-Window): Have this be per window instead of a crappy static variable
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
			spoopy_swapchain.metal.depth_stencil_texture = NULL;
			spoopy_swapchain.metal.msaa_color_texture = NULL;
			#endif

			break;
	}

	spoopy_swapchain.width  = fb_size.w;
	spoopy_swapchain.height = fb_size.h;
}

static inline sg_shader_stage spoopy_stage_to_sg(spoopy_shader_stage_t s) {
    switch (s) {
        case SPOOPY_STAGE_VERTEX:   return SG_SHADERSTAGE_VERTEX;
        case SPOOPY_STAGE_FRAGMENT: return SG_SHADERSTAGE_FRAGMENT;
        default:                    return SG_SHADERSTAGE_FRAGMENT;
    }
}

static inline sg_image_type spoopy_imagetype_from_params(const spoopy_texture_params_t* p) {
    switch (p->texture_class) {
        case SPOOPY_TEXTURE_CLASS_2D:
            return (p->layers > 1) ? SG_IMAGETYPE_ARRAY : SG_IMAGETYPE_2D;
        case SPOOPY_TEXTURE_CLASS_CUBE: return SG_IMAGETYPE_CUBE;
        default:
            return SG_IMAGETYPE_2D;
    }
}

static inline sg_image_sample_type spoopy_sample_type_from_format(spoopy_pixel_format_t fmt, bool is_depth_tex) {
	if(is_depth_tex) {
        return SG_IMAGESAMPLETYPE_DEPTH;
    }

	const bool is_float = spoopy_pixel_format_is_float(fmt);
    const uint32_t depth = spoopy_pixel_format_depth(fmt);
	const spoopy_pixel_layout_t layout = spoopy_format_layout(fmt);

	if(is_float) {
		// Sokol requires UNFILTERABLE for 32-bit float textures.
		if(depth == 32 &&
			(layout == SPOOPY_PIXEL_LAYOUT_R ||
			 layout == SPOOPY_PIXEL_LAYOUT_RG ||
			 layout == SPOOPY_PIXEL_LAYOUT_RGBA)) {
			return SG_IMAGESAMPLETYPE_UNFILTERABLE_FLOAT;
		}
		return SG_IMAGESAMPLETYPE_FLOAT;
	}

	// Normalized integer formats sample as float.
	return SG_IMAGESAMPLETYPE_FLOAT;
}

static inline sg_pixel_format spoopy_sokol_pixel_format(spoopy_pixel_format_t fmt, bool is_depth_tex) {
	if(is_depth_tex) {
		return SG_PIXELFORMAT_DEPTH;
	}

	const bool is_float = spoopy_pixel_format_is_float(fmt);
	const uint32_t depth = spoopy_pixel_format_depth(fmt);
	const spoopy_pixel_layout_t layout = spoopy_format_layout(fmt);

	switch(layout) {
		case SPOOPY_PIXEL_LAYOUT_R:
			if(is_float) {
				if(depth == 16) return SG_PIXELFORMAT_R16F;
				if(depth == 32) return SG_PIXELFORMAT_R32F;
			} else {
				if(depth == 8) return SG_PIXELFORMAT_R8;
				if(depth == 16) return SG_PIXELFORMAT_R16;
				if(depth == 32) return SG_PIXELFORMAT_R32UI;
			}
			break;
		case SPOOPY_PIXEL_LAYOUT_RG:
			if(is_float) {
				if(depth == 16) return SG_PIXELFORMAT_RG16F;
				if(depth == 32) return SG_PIXELFORMAT_RG32F;
			} else {
				if(depth == 8) return SG_PIXELFORMAT_RG8;
				if(depth == 16) return SG_PIXELFORMAT_RG16;
				if(depth == 32) return SG_PIXELFORMAT_RG32UI;
			}
			break;
		case SPOOPY_PIXEL_LAYOUT_RGB:
			SPOOPY_LOG_ERROR("RGB formats are not directly supported by Sokol (use RGBA).");
			return SG_PIXELFORMAT_NONE;
		case SPOOPY_PIXEL_LAYOUT_RGBA:
			if(is_float) {
				if(depth == 16) return SG_PIXELFORMAT_RGBA16F;
				if(depth == 32) return SG_PIXELFORMAT_RGBA32F;
			} else {
				if(depth == 8) return SG_PIXELFORMAT_RGBA8;
				if(depth == 16) return SG_PIXELFORMAT_RGBA16;
				if(depth == 32) return SG_PIXELFORMAT_RGBA32UI;
			}

			break;
		default:
			break;
	}

	SPOOPY_LOG_ERROR("Unsupported pixel format: layout=%u depth=%u float=%u",
		(unsigned)layout, (unsigned)depth, (unsigned)is_float);
	return SG_PIXELFORMAT_NONE;
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
	bool has_fragment_texture_binding = false;

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
				has_fragment_texture_binding =
					obj->func.source != NULL &&
					strstr(obj->func.source, "tex0") != NULL &&
					strstr(obj->func.source, "samp0") != NULL;
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

	if(has_fragment_texture_binding) {
		desc.views[0].texture = (sg_shader_texture_view) {
			.stage = SG_SHADERSTAGE_FRAGMENT,
			.image_type = SG_IMAGETYPE_2D,
			.sample_type = SG_IMAGESAMPLETYPE_FLOAT,
			.multisampled = false,
			.hlsl_register_t_n = 0,
			.msl_texture_n = 0,
			.wgsl_group1_binding_n = 0,
			.spirv_set1_binding_n = 0,
		};
		desc.samplers[0] = (sg_shader_sampler) {
			.stage = SG_SHADERSTAGE_FRAGMENT,
			.sampler_type = SG_SAMPLERTYPE_FILTERING,
			.hlsl_register_s_n = 0,
			.msl_sampler_n = 0,
			.wgsl_group1_binding_n = 1,
			.spirv_set1_binding_n = 1,
		};
		desc.texture_sampler_pairs[0] = (sg_shader_texture_sampler_pair) {
			.stage = SG_SHADERSTAGE_FRAGMENT,
			.view_slot = 0,
			.sampler_slot = 0,
		};
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

	const uint32_t wants_depth_clear = (flags & SPOOPY_BUFFER_DEPTH) != 0;
	const uint32_t has_depth_attachment = spoopy_swapchain.depth_format != SG_PIXELFORMAT_NONE;
	action.depth.load_action = has_depth_attachment
		? load_actions[wants_depth_clear]
		: SG_LOADACTION_DONTCARE;
	action.depth.clear_value = depth_val;

	spoopy_sokol_update_swapchain(graphics);

	sg_begin_pass(&(sg_pass) {
		.action = action,
		.swapchain = spoopy_swapchain,
	});
}

static void spoopy_sokol_pipeline_compile(spoopy_pipeline_t* pipeline, uint32_t spec_count, spoopy_vertex_attr_spec_t spec[spec_count], uint32_t buffer_index) {
	sg_pipeline_desc pdesc = {0};
	pdesc.shader = pipeline->shader;
	pdesc.index_type = SG_INDEXTYPE_UINT16;
	pdesc.color_count = 1;
	pdesc.sample_count = spoopy_swapchain.sample_count;
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

static inline spoopy_texture_filter_mode_t spoopy_linear_to_nearest(spoopy_texture_filter_mode_t filter) {
	switch(filter) {
		case SPOOPY_TEXTURE_FILTER_LINEAR:
			return SPOOPY_TEXTURE_FILTER_NEAREST;

		case SPOOPY_TEXTURE_FILTER_LINEAR_MIPMAP_LINEAR:
		case SPOOPY_TEXTURE_FILTER_LINEAR_MIPMAP_NEAREST:
			return SPOOPY_TEXTURE_FILTER_NEAREST_MIPMAP_NEAREST;

		default:
			return filter;
	}
}

static inline bool spoopy_sample_type_is_filterable(sg_image_sample_type st) {
	return st == SG_IMAGESAMPLETYPE_FLOAT;
}

static inline sg_filter spoopy_filter_to_sg_filter(spoopy_texture_filter_mode_t mode, bool filterable) {
	static const sg_filter map[] = {
		[SPOOPY_TEXTURE_FILTER_LINEAR] = SG_FILTER_LINEAR,
		[SPOOPY_TEXTURE_FILTER_LINEAR_MIPMAP_NEAREST] = SG_FILTER_LINEAR,
		[SPOOPY_TEXTURE_FILTER_LINEAR_MIPMAP_LINEAR] = SG_FILTER_LINEAR,
		[SPOOPY_TEXTURE_FILTER_NEAREST] = SG_FILTER_NEAREST,
		[SPOOPY_TEXTURE_FILTER_NEAREST_MIPMAP_NEAREST] = SG_FILTER_NEAREST,
		[SPOOPY_TEXTURE_FILTER_NEAREST_MIPMAP_LINEAR] = SG_FILTER_NEAREST,
	};

	assert((uint32_t)mode < sizeof(map) / sizeof(*map));
	sg_filter filter = map[mode];

	if(filterable) {
		return filter;
	}

	return map[spoopy_linear_to_nearest(mode)];
}

static inline sg_filter spoopy_filter_to_sg_mipmap_filter(spoopy_texture_filter_mode_t mode, bool filterable) {
	static const sg_filter map[] = {
		[SPOOPY_TEXTURE_FILTER_LINEAR] = SG_FILTER_NEAREST,
		[SPOOPY_TEXTURE_FILTER_LINEAR_MIPMAP_NEAREST] = SG_FILTER_NEAREST,
		[SPOOPY_TEXTURE_FILTER_LINEAR_MIPMAP_LINEAR] = SG_FILTER_LINEAR,
		[SPOOPY_TEXTURE_FILTER_NEAREST] = SG_FILTER_NEAREST,
		[SPOOPY_TEXTURE_FILTER_NEAREST_MIPMAP_NEAREST] = SG_FILTER_NEAREST,
		[SPOOPY_TEXTURE_FILTER_NEAREST_MIPMAP_LINEAR] = SG_FILTER_LINEAR,
	};

	assert((uint32_t)mode < sizeof(map) / sizeof(*map));
	sg_filter filter = map[mode];

	if(filterable) {
		return filter;
	}

	return map[spoopy_linear_to_nearest(mode)];
}

static inline sg_wrap spoopy_wrap_to_sg_wrap(spoopy_texture_wrap_mode_t mode) {
	static const sg_wrap map[] = {
		[SPOOPY_TEXTURE_WRAP_CLAMP] = SG_WRAP_CLAMP_TO_EDGE,
		[SPOOPY_TEXTURE_WRAP_MIRROR] = SG_WRAP_MIRRORED_REPEAT,
		[SPOOPY_TEXTURE_WRAP_REPEAT] = SG_WRAP_REPEAT,
	};

	assert((uint32_t)mode < sizeof(map) / sizeof(*map));
	return map[mode];
}

static inline sg_sampler_type spoopy_sampler_type_for_sample_type(sg_image_sample_type st, const spoopy_texture_params_t* p) {
	if (st == SG_IMAGESAMPLETYPE_UNFILTERABLE_FLOAT ||
        st == SG_IMAGESAMPLETYPE_UINT ||
        st == SG_IMAGESAMPLETYPE_SINT) {
        return SG_SAMPLERTYPE_NONFILTERING;
    }

	if (st == SG_IMAGESAMPLETYPE_DEPTH) {
        return SG_SAMPLERTYPE_NONFILTERING;
    }

	const bool filterable = spoopy_sample_type_is_filterable(st);
	const bool wants_linear =
		(spoopy_filter_to_sg_filter(p->filter.min, filterable) == SG_FILTER_LINEAR) ||
		(spoopy_filter_to_sg_filter(p->filter.mag, filterable) == SG_FILTER_LINEAR) ||
		(spoopy_filter_to_sg_mipmap_filter(p->filter.min, filterable) == SG_FILTER_LINEAR);

    return wants_linear ? SG_SAMPLERTYPE_FILTERING : SG_SAMPLERTYPE_NONFILTERING;
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
	memset(pipeline->bindings.vertex_buffers, 0, sizeof(pipeline->bindings.vertex_buffers));
	memset(pipeline->bindings.vertex_buffer_offsets, 0, sizeof(pipeline->bindings.vertex_buffer_offsets));
	pipeline->bindings.index_buffer = (sg_buffer){0};
	pipeline->bindings.index_buffer_offset = 0;

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

static size_t spoopy_sokol_texture_size(void) {
	return sizeof(spoopy_texture_t);
}

static void spoopy_sokol_texture_create(spoopy_texture_t* tex, const spoopy_texture_params_t* p) {
	const sg_shader_stage stage = spoopy_stage_to_sg(p->stage);
	memset(tex, 0, sizeof(*tex));
	tex->params = *p;

	tex->texture_view.stage        = stage;
	tex->texture_view.image_type   = spoopy_imagetype_from_params(p);
	tex->texture_view.sample_type  = spoopy_sample_type_from_format(p->format, p->depth_texture);
	tex->texture_view.multisampled = false;

	tex->sampler.stage = stage;
	tex->sampler.sampler_type = spoopy_sampler_type_for_sample_type(tex->texture_view.sample_type, p);

	const bool filterable = spoopy_sample_type_is_filterable(tex->texture_view.sample_type);
	sg_sampler_desc sampler_desc = {0};
	sampler_desc.min_filter = spoopy_filter_to_sg_filter(p->filter.min, filterable);
	sampler_desc.mag_filter = spoopy_filter_to_sg_filter(p->filter.mag, filterable);
	sampler_desc.mipmap_filter = spoopy_filter_to_sg_mipmap_filter(p->filter.min, filterable);
	sampler_desc.wrap_u = spoopy_wrap_to_sg_wrap(p->wrap.u);
	sampler_desc.wrap_v = spoopy_wrap_to_sg_wrap(p->wrap.v);
	sampler_desc.wrap_w = sampler_desc.wrap_u;

	tex->sampler_state = sg_make_sampler(&sampler_desc);
	if(tex->sampler_state.id == SG_INVALID_ID) {
		SPOOPY_LOG_ERROR("Failed to create Sokol sampler");
	}
}

static void spoopy_sokol_texture_fill(spoopy_texture_t* tex, uint32_t mipmap, uint32_t layer, const spoopy_image_t* img) {
	(void)layer; // TODO (Samples): Make use of this parameter by having support for dynamic textures

	assert(mipmap < SG_MAX_MIPMAPS);

	if(!tex || !img) {
		SPOOPY_LOG_ERROR("Invalid parameters for texture_fill");
		return;
	}

	// Create the image if it doesn't exist yet
	if(tex->image.id == SG_INVALID_ID) {
		sg_image_desc img_desc = { 0 };
		img_desc.type = tex->texture_view.image_type;
		img_desc.width = tex->params.width;
		img_desc.height = tex->params.height;
		img_desc.num_slices = tex->params.layers > 0 ? tex->params.layers : 1;
		img_desc.num_mipmaps = tex->params.mipmaps > 0 ? tex->params.mipmaps : 1;
		img_desc.pixel_format = spoopy_sokol_pixel_format(tex->params.format, tex->params.depth_texture);
		img_desc.usage.immutable = true;
		img_desc.usage.dynamic_update = false;

		// For immutable images, provide initial data during creation
		img_desc.data.mip_levels[mipmap].ptr = img->pixels.raw_data;
		img_desc.data.mip_levels[mipmap].size = img->data_size;

		tex->image = sg_make_image(&img_desc);
		if(tex->image.id == SG_INVALID_ID) {
			SPOOPY_LOG_ERROR("Failed to create Sokol image");
			return;
		}

		tex->view = sg_make_view(&(sg_view_desc) {
			.texture.image = tex->image,
		});
		if(tex->view.id == SG_INVALID_ID) {
			SPOOPY_LOG_ERROR("Failed to create Sokol texture view");
		}
	} else {
		SPOOPY_LOG_WARN("Cannot update immutable texture after creation");
	}
}

uint8_t spoopy_sokol_pipeline_get_texture_unit(spoopy_pipeline_t* pipeline, const char* name) {
	(void)pipeline;
	(void)name;
	return 0;
}

void spoopy_sokol_texture_set(spoopy_pipeline_t* pipeline, const char* u_tex, const char* u_samp, spoopy_texture_t* tex) {
	const uint8_t tex_slot = spoopy_api_get_bind_slot(u_tex);
	const uint8_t sampler_slot = spoopy_api_get_bind_slot(u_samp);

	if(!pipeline) {
		SPOOPY_LOG_WARN("Texture bind request ignored: pipeline is NULL");
		return;
	}

	if(tex_slot >= SG_MAX_VIEW_BINDSLOTS || sampler_slot >= SG_MAX_SAMPLER_BINDSLOTS) {
		SPOOPY_LOG_WARN("Texture bind request out of range: view=%u sampler=%u", tex_slot, sampler_slot);
		return;
	}

	if(!tex) {
		pipeline->bindings.views[tex_slot] = (sg_view){0};
		pipeline->bindings.samplers[sampler_slot] = (sg_sampler){0};
		return;
	}

	tex->texture_view.msl_texture_n = tex_slot;
	tex->sampler.msl_sampler_n = sampler_slot;
	pipeline->bindings.views[tex_slot] = tex->view;
	pipeline->bindings.samplers[sampler_slot] = tex->sampler_state;
}

static void spoopy_sokol_texture_destroy(spoopy_texture_t* tex) {
	if(!tex) {
		return;
	}

	if(tex->view.id != SG_INVALID_ID) {
		sg_destroy_view(tex->view);
	}
	if(tex->image.id != SG_INVALID_ID) {
		sg_destroy_image(tex->image);
	}
	if(tex->sampler_state.id != SG_INVALID_ID) {
		sg_destroy_sampler(tex->sampler_state);
	}

	*tex = (spoopy_texture_t){0};
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
	.texture_size = spoopy_sokol_texture_size,
	.texture_create = spoopy_sokol_texture_create,
	.texture_fill = spoopy_sokol_texture_fill,
	.texture_destroy = spoopy_sokol_texture_destroy,
	.pipeline_get_texture_unit = spoopy_sokol_pipeline_get_texture_unit,
	.texture_set = spoopy_sokol_texture_set,
};
