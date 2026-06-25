#include <spoopy_api.h>
#include <spoopy_backend.h>
#include <spoopy_reflect.h>
#include <spoopy_image.h>
#include <utils/assert.h>

#define SPOOPY_SOKOL_IMPLEMENTATION
#include "spoopy_sokol.h"

const size_t spoopy_shader_object_size = sizeof(spoopy_shader_object_t);

static_assert(sizeof(spoopy_color_t) >= sizeof(sg_color), "spoopy_color_t must hold sg_color");
static_assert(__alignof(spoopy_color_t) >= __alignof(sg_color), "spoopy_color_t alignment must satisfy sg_color");

spoopy_sokol_global_t spoopy_sokol = {
	.frame = {
		.swapchain = {
			.color_format = SG_PIXELFORMAT_BGRA8,
			.depth_format = SG_PIXELFORMAT_NONE,
			.sample_count = 1,
		},
	},
};

static inline spoopy_capability_bits_t spoopy_sokol_capability_bit(spoopy_render_capability_t cap) {
	const spoopy_capability_bits_t idx = (spoopy_capability_bits_t)cap;
	assert(idx < SPOOPY_NUM_RCAPS);
	return (spoopy_capability_bits_t)(1u << idx);
}

static inline sg_blend_op spoopy_sokol_blend_op(spoopy_blend_op_t op) {
	switch(op) {
		case SPOOPY_BLENDOP_ADD:     return SG_BLENDOP_ADD;
		case SPOOPY_BLENDOP_SUB:     return SG_BLENDOP_SUBTRACT;
		case SPOOPY_BLENDOP_REV_SUB: return SG_BLENDOP_REVERSE_SUBTRACT;
		case SPOOPY_BLENDOP_MIN:     return SG_BLENDOP_MIN;
		case SPOOPY_BLENDOP_MAX:     return SG_BLENDOP_MAX;
		default:                     return SG_BLENDOP_ADD;
	}
}

static inline sg_blend_factor spoopy_sokol_blend_factor(spoopy_blend_factor_t factor) {
	switch(factor) {
		case SPOOPY_BLENDFACTOR_ZERO:          return SG_BLENDFACTOR_ZERO;
		case SPOOPY_BLENDFACTOR_ONE:           return SG_BLENDFACTOR_ONE;
		case SPOOPY_BLENDFACTOR_SRC_COLOR:     return SG_BLENDFACTOR_SRC_COLOR;
		case SPOOPY_BLENDFACTOR_INV_SRC_COLOR: return SG_BLENDFACTOR_ONE_MINUS_SRC_COLOR;
		case SPOOPY_BLENDFACTOR_SRC_ALPHA:     return SG_BLENDFACTOR_SRC_ALPHA;
		case SPOOPY_BLENDFACTOR_INV_SRC_ALPHA: return SG_BLENDFACTOR_ONE_MINUS_SRC_ALPHA;
		case SPOOPY_BLENDFACTOR_DST_COLOR:     return SG_BLENDFACTOR_DST_COLOR;
		case SPOOPY_BLENDFACTOR_INV_DST_COLOR: return SG_BLENDFACTOR_ONE_MINUS_DST_COLOR;
		case SPOOPY_BLENDFACTOR_DST_ALPHA:     return SG_BLENDFACTOR_DST_ALPHA;
		case SPOOPY_BLENDFACTOR_INV_DST_ALPHA: return SG_BLENDFACTOR_ONE_MINUS_DST_ALPHA;
		default:                               return SG_BLENDFACTOR_ONE;
	}
}

static inline sg_cull_mode spoopy_sokol_cull_mode(const spoopy_pipeline_t* pipeline) {
	if(!(pipeline->state.caps & spoopy_sokol_capability_bit(SPOOPY_RCAP_CULL_FACE))) {
		return SG_CULLMODE_NONE;
	}

	switch(pipeline->state.cull) {
		case SPOOPY_CULL_FRONT:
			return SG_CULLMODE_FRONT;
		case SPOOPY_CULL_BACK:
		case SPOOPY_CULL_BOTH:
		default:
			return SG_CULLMODE_BACK;
	}
}

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
			#if defined(SOKOL_METAL)
			spoopy_sokol.frame.swapchain.metal.current_drawable = spoopy_graphics_get_native_drawable(graphics);
			spoopy_sokol.frame.swapchain.metal.depth_stencil_texture = NULL;
			spoopy_sokol.frame.swapchain.metal.msaa_color_texture = NULL;
			#endif

			break;
	}

	spoopy_sokol.frame.swapchain.width  = fb_size.w;
	spoopy_sokol.frame.swapchain.height = fb_size.h;
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

static inline bool spoopy_sokol_uniform_is_sampler(spoopy_uniform_type_t type) {
	return type == SPOOPY_UNIFORM_SAMPLER_2D || type == SPOOPY_UNIFORM_SAMPLER_CUBE;
}

static inline size_t spoopy_sokol_base_type_size(spoopy_shader_base_type_t base_type) {
	switch(base_type) {
		case SPOOPY_SHADER_BASE_TYPE_BOOLEAN:
		case SPOOPY_SHADER_BASE_TYPE_INT32:
		case SPOOPY_SHADER_BASE_TYPE_UINT32:
		case SPOOPY_SHADER_BASE_TYPE_FP32:
			return 4;
		case SPOOPY_SHADER_BASE_TYPE_INT16:
		case SPOOPY_SHADER_BASE_TYPE_UINT16:
		case SPOOPY_SHADER_BASE_TYPE_FP16:
			return 2;
		case SPOOPY_SHADER_BASE_TYPE_INT8:
		case SPOOPY_SHADER_BASE_TYPE_UINT8:
			return 1;
		case SPOOPY_SHADER_BASE_TYPE_INT64:
		case SPOOPY_SHADER_BASE_TYPE_UINT64:
		case SPOOPY_SHADER_BASE_TYPE_FP64:
			return 8;
		default:
			return 0;
	}
}

static inline spoopy_uniform_type_t spoopy_sokol_uniform_type_from_data_type(const spoopy_data_type_t* type) {
	if(!type) {
		return SPOOPY_UNIFORM_UNKNOWN;
	}

	if(type->matrix_columns == 3 && type->vector_size == 3) {
		return SPOOPY_UNIFORM_MAT3;
	}

	if(type->matrix_columns == 4 && type->vector_size == 4) {
		return SPOOPY_UNIFORM_MAT4;
	}

	const bool is_float =
		type->base_type == SPOOPY_SHADER_BASE_TYPE_FP16 ||
		type->base_type == SPOOPY_SHADER_BASE_TYPE_FP32 ||
		type->base_type == SPOOPY_SHADER_BASE_TYPE_FP64;
	const bool is_int =
		type->base_type == SPOOPY_SHADER_BASE_TYPE_BOOLEAN ||
		type->base_type == SPOOPY_SHADER_BASE_TYPE_INT8 ||
		type->base_type == SPOOPY_SHADER_BASE_TYPE_UINT8 ||
		type->base_type == SPOOPY_SHADER_BASE_TYPE_INT16 ||
		type->base_type == SPOOPY_SHADER_BASE_TYPE_UINT16 ||
		type->base_type == SPOOPY_SHADER_BASE_TYPE_INT32 ||
		type->base_type == SPOOPY_SHADER_BASE_TYPE_UINT32;

	if(is_float) {
		switch(type->vector_size) {
			case 1: return SPOOPY_UNIFORM_FLOAT;
			case 2: return SPOOPY_UNIFORM_VEC2;
			case 3: return SPOOPY_UNIFORM_VEC3;
			case 4: return SPOOPY_UNIFORM_VEC4;
			default: return SPOOPY_UNIFORM_UNKNOWN;
		}
	}

	if(is_int) {
		switch(type->vector_size) {
			case 1: return SPOOPY_UNIFORM_INT;
			case 2: return SPOOPY_UNIFORM_IVEC2;
			case 3: return SPOOPY_UNIFORM_IVEC3;
			case 4: return SPOOPY_UNIFORM_IVEC4;
			default: return SPOOPY_UNIFORM_UNKNOWN;
		}
	}

	return SPOOPY_UNIFORM_UNKNOWN;
}

static inline spoopy_uniform_type_t spoopy_sokol_uniform_type_from_sampler_type(const ShaderSamplerType* type) {
	if(!type) {
		return SPOOPY_UNIFORM_UNKNOWN;
	}

	if(type->flags & (SHADER_SAMPLER_DEPTH | SHADER_SAMPLER_ARRAYED | SHADER_SAMPLER_MULTISAMPLED)) {
		return SPOOPY_UNIFORM_UNKNOWN;
	}

	switch(type->dim) {
		case SPOOPY_SHADER_SAMPLER_DIM_2D:
			return SPOOPY_UNIFORM_SAMPLER_2D;
		case SPOOPY_SHADER_SAMPLER_DIM_CUBE:
			return SPOOPY_UNIFORM_SAMPLER_CUBE;
		default:
			return SPOOPY_UNIFORM_UNKNOWN;
	}
}

static inline sg_image_type spoopy_sokol_image_type_from_sampler_type(const ShaderSamplerType* type) {
	if(type && type->dim == SPOOPY_SHADER_SAMPLER_DIM_CUBE) {
		return SG_IMAGETYPE_CUBE;
	}

	return SG_IMAGETYPE_2D;
}

static inline sg_image_sample_type spoopy_sokol_sample_type_from_sampler_type(const ShaderSamplerType* type) {
	if(type && (type->flags & SHADER_SAMPLER_DEPTH)) {
		return SG_IMAGESAMPLETYPE_DEPTH;
	}

	return SG_IMAGESAMPLETYPE_FLOAT;
}

static inline sg_sampler_type spoopy_sokol_sampler_type_from_sampler_type(const ShaderSamplerType* type) {
	if(type && (type->flags & SHADER_SAMPLER_DEPTH)) {
		return SG_SAMPLERTYPE_COMPARISON;
	}

	return SG_SAMPLERTYPE_FILTERING;
}

static inline bool spoopy_sokol_bindslot_set_texture_view(sg_shader_texture_view* view, uint16_t binding) {
#if defined(SOKOL_METAL)
	view->msl_texture_n = (uint8_t)binding;
	return true;
#elif defined(SOKOL_WGPU)
	view->wgsl_group1_binding_n = (uint8_t)binding;
	return true;
#else
	(void)view;
	(void)binding;
	SPOOPY_LOG_ERROR("Reflection-driven texture bindings are only implemented for Metal and WGPU.");
	return false;
#endif
}

static inline bool spoopy_sokol_bindslot_set_sampler(sg_shader_sampler* sampler, uint16_t binding) {
#if defined(SOKOL_METAL)
	sampler->msl_sampler_n = (uint8_t)binding;
	return true;
#elif defined(SOKOL_WGPU)
	sampler->wgsl_group1_binding_n = (uint8_t)binding;
	return true;
#else
	(void)sampler;
	(void)binding;
	SPOOPY_LOG_ERROR("Reflection-driven sampler bindings are only implemented for Metal and WGPU.");
	return false;
#endif
}

static inline bool spoopy_sokol_bindslot_set_uniform_block(sg_shader_uniform_block* block, uint16_t binding) {
#if defined(SOKOL_METAL)
	block->msl_buffer_n = (uint8_t)binding;
	return true;
#elif defined(SOKOL_WGPU)
	block->wgsl_group0_binding_n = (uint8_t)binding;
	return true;
#else
	(void)block;
	(void)binding;
	SPOOPY_LOG_ERROR("Reflection-driven uniform bindings are only implemented for Metal and WGPU.");
	return false;
#endif
}

static inline bool spoopy_sokol_texture_set_binding_indices(spoopy_texture_t* tex, uint16_t view_binding, uint16_t sampler_binding) {
	if(!tex) {
		return false;
	}

	return spoopy_sokol_bindslot_set_texture_view(&tex->texture_view, view_binding) &&
		spoopy_sokol_bindslot_set_sampler(&tex->sampler, sampler_binding);
}

static bool spoopy_shader_object_init_uniforms(spoopy_shader_object_t* shader, const spoopy_shader_reflection_t* reflection) {
	size_t uniform_count = 0;
	spoopy_uniform_t* texture_uniforms[reflection ? reflection->num_samplers : 1];
	uint16_t sampler_bindings[reflection ? reflection->num_samplers : 1];
	uint16_t texture_count = 0;
	uint16_t sampler_count = 0;
	const spoopy_shader_block_t* selected_block = NULL;

	if(!reflection) {
		return false;
	}

	for(uint16_t i = 0; i < reflection->num_uniform_buffers; ++i) {
		selected_block = &reflection->uniform_buffers[i];
		uniform_count += selected_block->num_fields;
		break;
	}

	for(uint16_t i = 0; i < reflection->num_samplers; ++i) {
		if(spoopy_sokol_uniform_type_from_sampler_type(&reflection->samplers[i].type) != SPOOPY_UNIFORM_UNKNOWN) {
			++uniform_count;
		}
	}

	spoopy_uniform_vec_init(&shader->uniforms, uniform_count ? uniform_count : 1);

	if(selected_block) {
		const spoopy_shader_block_t* block = selected_block;

		shader->uniform_buffer.data = spoopy_heap_alloc(block->size ? block->size : 1);
		if(!shader->uniform_buffer.data) {
			SPOOPY_LOG_ERROR("Failed to allocate uniform buffer of %u bytes", block->size);
			return false;
		}

		memset(shader->uniform_buffer.data, 0, block->size);
		shader->uniform_buffer.size = block->size;
		shader->uniform_buffer.binding = block->binding;
		shader->uniform_buffer.slot = (shader->stage == SPOOPY_STAGE_VERTEX) ? 0 : 1;

		for(uint16_t j = 0; j < block->num_fields; ++j) {
			const spoopy_shader_struct_field_t* field = &block->fields[j];
			const spoopy_uniform_type_t uniform_type = spoopy_sokol_uniform_type_from_data_type(&field->type);

			if(uniform_type == SPOOPY_UNIFORM_UNKNOWN || !field->name) {
				continue;
			}

			spoopy_uniform_vec_add(&shader->uniforms, ((spoopy_uniform_t) {
				.name = spoopy_arena_strdup(&shader->arena, field->name),
				.hash = 0,
				.type = uniform_type,
				.buffer_backed = {
					.data = shader->uniform_buffer.data,
					.offset = field->offset,
					.data_type = field->type,
				},
			}));
		}
	}

	for(uint16_t i = 1; i < reflection->num_uniform_buffers; ++i) {
		const spoopy_shader_block_t* block = &reflection->uniform_buffers[i];
		SPOOPY_LOG_WARN("Ignoring extra uniform block '%s' on stage %d",
			block->name ? block->name : "(unnamed)",
			shader->stage);
	}

	for(uint16_t i = 0; i < reflection->num_samplers; ++i) {
		const spoopy_shader_sampler_t* sampler = &reflection->samplers[i];
		const spoopy_uniform_type_t uniform_type = spoopy_sokol_uniform_type_from_sampler_type(&sampler->type);

		if(uniform_type != SPOOPY_UNIFORM_UNKNOWN) {
			spoopy_uniform_vec_add(&shader->uniforms, ((spoopy_uniform_t) {
				.name = spoopy_arena_strdup(&shader->arena, sampler->name),
				.hash = 0,
				.type = uniform_type,
				.sampler = {
					.binding = sampler->binding,
					.paired_binding = UINT16_MAX,
					.sampler_type = sampler->type,
				},
			}));

			texture_uniforms[texture_count++] = &shader->uniforms.data[shader->uniforms.capacity - 1];
			continue;
		}

		if(sampler->type.dim == SPOOPY_SHADER_SAMPLER_DIM_UNKNOWN && sampler_count < reflection->num_samplers) {
			sampler_bindings[sampler_count++] = sampler->binding;
		}
	}

	const uint16_t pair_count = texture_count < sampler_count ? texture_count : sampler_count;
	for(uint16_t i = 0; i < pair_count; ++i) {
		texture_uniforms[i]->sampler.paired_binding = sampler_bindings[i];
		// Temporary debug logging for texture/sampler reflection pairing.
		SPOOPY_LOG_INFO(
			"Texture/sampler pair: stage=%d uniform='%s' texture_binding=%u sampler_binding=%u",
			shader->stage,
			texture_uniforms[i]->name ? texture_uniforms[i]->name : "(unnamed)",
			texture_uniforms[i]->sampler.binding,
			texture_uniforms[i]->sampler.paired_binding
		);
	}

	if(texture_count != sampler_count) {
		SPOOPY_LOG_WARN("Stage %d has %u texture bindings and %u sampler bindings; pairing %u",
			shader->stage,
			texture_count,
			sampler_count,
			pair_count);
	}

	return true;
}

static void spoopy_sokol_shader_destroy(spoopy_shader_object_t* shader, bool must_free) {
	if(!shader) {
		SPOOPY_LOG_WARN("`shader` provided was (null)!");
		return;
	}

	(void)must_free;
	spoopy_uniform_vec_destroy(&shader->uniforms);

	if(shader->uniform_buffer.data) {
		spoopy_heap_free(shader->uniform_buffer.data);
		shader->uniform_buffer.data = NULL;
	}

	if(shader->arena.pages.begin_page) {
		spoopy_arena_deinit(&shader->arena);
	}
}

static bool spoopy_sokol_shader_init(spoopy_shader_object_t* shader, spoopy_shader_source_t* info) {
	char* source_copy = NULL;
	char* entry_copy = NULL;

	if(SPOOPY_UNLIKELY(!spoopy_api_shader_supported(info, NULL))) {
		SPOOPY_LOG_ERROR("Shading language not supported!");
		return false;
	}

	const spoopy_shader_reflection_t* reflection = info->reflection;

	if(!reflection) {
		SPOOPY_LOG_ERROR("Shader has no reflection data!");
		return false;
	}

	*shader = (spoopy_shader_object_t){0};
	shader->stage = info->stage;

	const spoopy_renderer_t renderer = spoopy_graphics_pick_renderer(info->target);
	if(!spoopy_graphics_renderer_is_single(renderer)) {
		SPOOPY_LOG_ERROR("Cannot support multiple renderers!");
		return false;
	}

	size_t src_len = info->content_size > 0 ? info->content_size : strlen(info->content);
	size_t entry_len = strlen(info->entry_point);
	spoopy_arena_init(&shader->arena, (src_len + entry_len + 2));
	if(info->content) {
		source_copy = spoopy_arena_alloc(&shader->arena, src_len + 1);
		if(!source_copy) {
			SPOOPY_LOG_ERROR("Out of memory copying shader source");
			goto fail;
		}

		memcpy(source_copy, info->content, src_len);
		source_copy[src_len] = '\0';
		shader->func.source = source_copy;
	}

	if(info->entry_point) {
		entry_copy = spoopy_arena_alloc(&shader->arena, entry_len + 1);
		if(!entry_copy) {
			SPOOPY_LOG_ERROR("Out of memory copying shader entry point");
			goto fail;
		}

		memcpy(entry_copy, info->entry_point, entry_len + 1);
		shader->func.entry = entry_copy;
	}

	if(!spoopy_shader_object_init_uniforms(shader, reflection)) {
		goto fail;
	}

	return true;

fail:
	spoopy_sokol_shader_destroy(shader, true);
	*shader = (spoopy_shader_object_t){0};
	return false;
}

static void spoopy_sokol_pipeline_free_allocations(spoopy_pipeline_t* pipeline) {
	if(!pipeline) {
		return;
	}

	if(pipeline->pipeline.id != SG_INVALID_ID) {
		sg_destroy_pipeline(pipeline->pipeline);
	}

	if(pipeline->shader.id != SG_INVALID_ID) {
		sg_destroy_shader(pipeline->shader);
	}

	if(pipeline->arena.pages.begin_page) {
		spoopy_arena_deinit(&pipeline->arena);
	}

	*pipeline = (spoopy_pipeline_t){0};
}

static size_t spoopy_sokol_pipeline_arena_size(uint32_t num_objs, spoopy_shader_object_t* objs[], size_t total_uniforms) {
	size_t size = spoopy_uniform_ht_capacity(total_uniforms) * sizeof(spoopy_uniform_ht_entry_t);

	for(uint32_t i = 0; i < num_objs; ++i) {
		const spoopy_shader_object_t* obj = objs[i];
		if(!obj) {
			continue;
		}

		for(size_t j = 0; j < obj->uniforms.capacity; ++j) {
			const spoopy_uniform_t* uniform = &obj->uniforms.data[j];
			if(uniform->name) {
				size += strlen(uniform->name) + 1;
			}
		}
	}

	return spoopy_max(size, (size_t)256);
}

static spoopy_pipeline_t* spoopy_sokol_pipeline_link(uint32_t num_objs, spoopy_shader_object_t* objs[]) {
	if(num_objs == 0 || !objs) {
		SPOOPY_LOG_ERROR("No shader objects provided for pipeline link");
		return NULL;
	}

	size_t total_uniforms = 0;
	for(uint32_t i = 0; i < num_objs; ++i) {
		if(objs[i]) {
			total_uniforms += objs[i]->uniforms.capacity;
		}
	}

	sg_shader_desc desc = {0};
	bool has_vertex = false;
	bool has_fragment = false;
	uint32_t num_pairs = 0;

	spoopy_pipeline_t* pipeline = spoopy_heap_alloc(sizeof(*pipeline));
	assert(pipeline);
	*pipeline = (spoopy_pipeline_t){
		.state = {
			.blend = SPOOPY_BLEND_NONE,
			.cull = SPOOPY_CULL_BACK,
		},
	};
	spoopy_arena_init(&pipeline->arena, spoopy_sokol_pipeline_arena_size(num_objs, objs, total_uniforms));
	if(!spoopy_uniform_ht_init(&pipeline->uniforms, &pipeline->arena, total_uniforms)) {
		goto fail;
	}

	for(uint32_t i=0; i<num_objs; i++) {
		spoopy_shader_object_t* obj = objs[i];
		const sg_shader_stage stage = spoopy_stage_to_sg(obj ? obj->stage : SPOOPY_STAGE_INVALID);

		if(!obj) {
			continue;
		}

		switch(obj->stage) {
			case SPOOPY_STAGE_VERTEX:
				pipeline->shader_stages.vertex = obj;
				desc.vertex_func = obj->func;
				has_vertex = true;
				SPOOPY_LOG_INFO("Vertex shader entry: '%s', source len: %zu",
					obj->func.entry ? obj->func.entry : "(null)",
					obj->func.source ? strlen(obj->func.source) : 0);
				break;
			case SPOOPY_STAGE_FRAGMENT:
				pipeline->shader_stages.fragment = obj;
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

		if(obj->uniform_buffer.data && obj->uniform_buffer.size) {
			sg_shader_uniform_block* block = &desc.uniform_blocks[obj->uniform_buffer.slot];
			*block = (sg_shader_uniform_block) {
				.stage = stage,
				.size = obj->uniform_buffer.size,
				.layout = SG_UNIFORMLAYOUT_NATIVE,
			};

			if(!spoopy_sokol_bindslot_set_uniform_block(block, obj->uniform_buffer.binding)) {
				goto fail;
			}
		}

		for(size_t j = 0; j < obj->uniforms.capacity; ++j) {
			const spoopy_uniform_t* uniform = &obj->uniforms.data[j];

			if(spoopy_sokol_uniform_is_sampler(uniform->type)) {
				if(uniform->sampler.paired_binding == UINT16_MAX) {
					SPOOPY_LOG_WARN("Texture uniform '%s' has no paired sampler binding", uniform->name);
					continue;
				}

				if(uniform->sampler.binding >= SG_MAX_VIEW_BINDSLOTS ||
				   uniform->sampler.paired_binding >= SG_MAX_SAMPLER_BINDSLOTS ||
				   num_pairs >= SG_MAX_TEXTURE_SAMPLER_PAIRS) {
					SPOOPY_LOG_ERROR("Sampler binding out of range for '%s'", uniform->name);
					goto fail;
				}

				desc.views[uniform->sampler.binding].texture = (sg_shader_texture_view) {
					.stage = stage,
					.image_type = spoopy_sokol_image_type_from_sampler_type(&uniform->sampler.sampler_type),
					.sample_type = spoopy_sokol_sample_type_from_sampler_type(&uniform->sampler.sampler_type),
					.multisampled = (uniform->sampler.sampler_type.flags & SHADER_SAMPLER_MULTISAMPLED) != 0,
				};
				if(!spoopy_sokol_bindslot_set_texture_view(&desc.views[uniform->sampler.binding].texture, uniform->sampler.binding)) {
					goto fail;
				}

				desc.samplers[uniform->sampler.paired_binding] = (sg_shader_sampler) {
					.stage = stage,
					.sampler_type = spoopy_sokol_sampler_type_from_sampler_type(&uniform->sampler.sampler_type),
				};
				if(!spoopy_sokol_bindslot_set_sampler(&desc.samplers[uniform->sampler.paired_binding], uniform->sampler.paired_binding)) {
					goto fail;
				}

				desc.texture_sampler_pairs[num_pairs++] = (sg_shader_texture_sampler_pair) {
					.stage = stage,
					.view_slot = uniform->sampler.binding,
					.sampler_slot = uniform->sampler.paired_binding,
				};
			}

			if(!spoopy_uniform_ht_insert_copy(&pipeline->uniforms, &pipeline->arena, uniform, obj->uniform_buffer.data)) {
				goto fail;
			}
		}
	}

	if(!has_vertex || !has_fragment) {
		SPOOPY_LOG_ERROR("Pipeline link requires both vertex and fragment shaders");
		goto fail;
	}
	pipeline->shader = sg_make_shader(&desc);

	SPOOPY_LOG_INFO("Created shader with id=%u", pipeline->shader.id);

	if(pipeline->shader.id == SG_INVALID_ID) {
		SPOOPY_LOG_ERROR("Failed to create Sokol shader");
		goto fail;
	}

	return pipeline;

fail:
	spoopy_sokol_pipeline_free_allocations(pipeline);
	spoopy_heap_free(pipeline);
	return NULL;
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

static size_t spoopy_sokol_vertex_attr_size(const spoopy_vertex_attr_spec_t* spec) {
	size_t element_size = 0;

	switch(spec->type) {
		case SPOOPY_VA_FLOAT:
		case SPOOPY_VA_INT:
		case SPOOPY_VA_UINT:
			element_size = 4;
			break;

		case SPOOPY_VA_SHORT:
		case SPOOPY_VA_USHORT:
			element_size = 2;
			break;

		case SPOOPY_VA_BYTE:
		case SPOOPY_VA_UBYTE:
			element_size = 1;
			break;

		default:
			SPOOPY_LOG_ERROR("Invalid vertex attribute type: %u", spec->type);
			return 0;
	}

	return element_size * spec->elements;
}

static void spoopy_sokol_clear(spoopy_graphics_t* graphics, spoopy_buffer_kind_t flags, const spoopy_color_t *color_val, float depth_val) {
	static const sg_load_action load_actions[2] = { SG_LOADACTION_LOAD, SG_LOADACTION_CLEAR };

	sg_pass_action action = {0};
	const uint32_t color_clear = (flags & SPOOPY_BUFFER_COLOR) != 0;
	action.colors[0].load_action = load_actions[color_clear];
	action.colors[0].clear_value = *(const sg_color*)color_val->rgba;

	const uint32_t wants_depth_clear = (flags & SPOOPY_BUFFER_DEPTH) != 0;
	const uint32_t has_depth_attachment = spoopy_sokol.frame.swapchain.depth_format != SG_PIXELFORMAT_NONE;
	action.depth.load_action = has_depth_attachment
		? load_actions[wants_depth_clear]
		: SG_LOADACTION_DONTCARE;
	action.depth.clear_value = depth_val;

	spoopy_sokol_update_swapchain(graphics);

	sg_begin_pass(&(sg_pass) {
		.action = action,
		.swapchain = spoopy_sokol.frame.swapchain,
	});
}

static void spoopy_sokol_pipeline_apply_desc(spoopy_pipeline_t* pipeline) {
	sg_pipeline_desc pdesc = {0};
	size_t stride = 0;
	const uint32_t spec_count = pipeline->vertex_spec_count;
	const uint32_t buffer_index = pipeline->vertex_buffer_index;
	const bool has_depth_attachment = spoopy_sokol.frame.swapchain.depth_format != SG_PIXELFORMAT_NONE;
	const spoopy_capability_bits_t caps = pipeline->state.caps;
	const spoopy_blend_mode_t blend = pipeline->state.blend;

	pdesc.shader = pipeline->shader;
	pdesc.index_type = SG_INDEXTYPE_UINT16;
	pdesc.color_count = 1;
	pdesc.sample_count = spoopy_sokol.frame.swapchain.sample_count;
	pdesc.colors[0].pixel_format = spoopy_sokol.frame.swapchain.color_format;
	pdesc.colors[0].blend.enabled = blend != SPOOPY_BLEND_NONE;
	pdesc.colors[0].blend.src_factor_rgb = spoopy_sokol_blend_factor(
		(spoopy_blend_factor_t)SPOOPY_BLENDMODE_COMPONENT(blend, SPOOPY_BLENDCOMP_SRC_COLOR));
	pdesc.colors[0].blend.dst_factor_rgb = spoopy_sokol_blend_factor(
		(spoopy_blend_factor_t)SPOOPY_BLENDMODE_COMPONENT(blend, SPOOPY_BLENDCOMP_DST_COLOR));
	pdesc.colors[0].blend.op_rgb = spoopy_sokol_blend_op(
		(spoopy_blend_op_t)SPOOPY_BLENDMODE_COMPONENT(blend, SPOOPY_BLENDCOMP_COLOR_OP));
	pdesc.colors[0].blend.src_factor_alpha = spoopy_sokol_blend_factor(
		(spoopy_blend_factor_t)SPOOPY_BLENDMODE_COMPONENT(blend, SPOOPY_BLENDCOMP_SRC_ALPHA));
	pdesc.colors[0].blend.dst_factor_alpha = spoopy_sokol_blend_factor(
		(spoopy_blend_factor_t)SPOOPY_BLENDMODE_COMPONENT(blend, SPOOPY_BLENDCOMP_DST_ALPHA));
	pdesc.colors[0].blend.op_alpha = spoopy_sokol_blend_op(
		(spoopy_blend_op_t)SPOOPY_BLENDMODE_COMPONENT(blend, SPOOPY_BLENDCOMP_ALPHA_OP));
	pdesc.depth.pixel_format = spoopy_sokol.frame.swapchain.depth_format;
	pdesc.depth.compare = ((caps & spoopy_sokol_capability_bit(SPOOPY_RCAP_DEPTH_TEST)) && has_depth_attachment)
		? SG_COMPAREFUNC_LESS_EQUAL
		: SG_COMPAREFUNC_ALWAYS;
	pdesc.depth.write_enabled = (caps & spoopy_sokol_capability_bit(SPOOPY_RCAP_DEPTH_WRITE)) && has_depth_attachment;
	pdesc.cull_mode = spoopy_sokol_cull_mode(pipeline);

	for(uint32_t i = 0; i < spec_count && i < SG_MAX_VERTEX_ATTRIBUTES; i++) {
		const size_t attr_size = spoopy_sokol_vertex_attr_size(&pipeline->vertex_spec[i]);
		pdesc.layout.attrs[i].format = spoopy_sokol_vertex_format(&pipeline->vertex_spec[i]);
		pdesc.layout.attrs[i].buffer_index = (int)buffer_index;
		pdesc.layout.attrs[i].offset = (int)stride;
		stride += attr_size;
	}

	if(buffer_index < SG_MAX_VERTEXBUFFER_BINDSLOTS) {
		pdesc.layout.buffers[buffer_index].stride = (int)stride;
	}

	if(pipeline->pipeline.id != SG_INVALID_ID) {
		sg_destroy_pipeline(pipeline->pipeline);
	}

	pipeline->pipeline = sg_make_pipeline(&pdesc);
}

static void spoopy_sokol_pipeline_compile(spoopy_pipeline_t* pipeline, uint32_t spec_count, spoopy_vertex_attr_spec_t spec[spec_count], uint32_t buffer_index) {
	if(!pipeline) {
		return;
	}

	pipeline->vertex_spec_count = spec_count > SG_MAX_VERTEX_ATTRIBUTES ? SG_MAX_VERTEX_ATTRIBUTES : spec_count;
	pipeline->vertex_buffer_index = buffer_index;

	for(uint32_t i = 0; i < pipeline->vertex_spec_count; ++i) {
		pipeline->vertex_spec[i] = spec[i];
	}

	spoopy_sokol_pipeline_apply_desc(pipeline);
}

static void spoopy_sokol_capabilities(spoopy_pipeline_t* pipeline, spoopy_capability_bits_t new_caps) {
	assert(pipeline);
	pipeline->state.caps = new_caps;
	spoopy_sokol_pipeline_apply_desc(pipeline);
}

static spoopy_capability_bits_t spoopy_sokol_capabilities_current(spoopy_pipeline_t* pipeline) {
	assert(pipeline);
	return pipeline->state.caps;
}

static void spoopy_sokol_blend(spoopy_pipeline_t* pipeline, spoopy_blend_mode_t mode) {
	assert(pipeline);
	pipeline->state.blend = mode;
	spoopy_sokol_pipeline_apply_desc(pipeline);
}

static spoopy_blend_mode_t spoopy_sokol_blend_current(spoopy_pipeline_t* pipeline) {
	assert(pipeline);
	return pipeline->state.blend;
}

static void spoopy_sokol_cull(spoopy_pipeline_t* pipeline, spoopy_cull_face_mode_t mode) {
	assert(pipeline);
	pipeline->state.cull = mode;
	spoopy_sokol_pipeline_apply_desc(pipeline);
}

static spoopy_cull_face_mode_t spoopy_sokol_cull_current(spoopy_pipeline_t* pipeline) {
	assert(pipeline);
	return pipeline->state.cull;
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

static inline uint8_t* spoopy_sokol_uniform_dst(spoopy_uniform_t* uniform) {
	if(!uniform || spoopy_sokol_uniform_is_sampler(uniform->type) || !uniform->buffer_backed.data) {
		return NULL;
	}

	return uniform->buffer_backed.data + uniform->buffer_backed.offset;
}

static size_t spoopy_sokol_uniform_storage_size(const spoopy_data_type_t* type) {
	const size_t scalar_size = spoopy_sokol_base_type_size(type ? type->base_type : SPOOPY_SHADER_BASE_TYPE_UNKNOWN);
	const size_t vector_size = (type && type->vector_size) ? type->vector_size : 1;

	if(!scalar_size || !type) {
		return 0;
	}

	if(type->array_size > 1 && type->array_stride > 0) {
		return (size_t)type->array_size * type->array_stride;
	}

	if(type->matrix_columns > 1) {
		const size_t column_size = scalar_size * vector_size;
		const size_t stride = type->matrix_stride ? type->matrix_stride : column_size;
		return (size_t)type->matrix_columns * stride;
	}

	return scalar_size * vector_size;
}

static void spoopy_sokol_uniform_write_bytes(spoopy_uniform_t* uniform, const void* src, size_t src_size) {
	uint8_t* dst = spoopy_sokol_uniform_dst(uniform);
	if(!dst || !src) {
		return;
	}

	const size_t storage_size = spoopy_sokol_uniform_storage_size(&uniform->buffer_backed.data_type);
	const size_t copy_size = src_size < storage_size ? src_size : storage_size;
	memcpy(dst, src, copy_size);
}

static void spoopy_sokol_uniform_write_matrix(spoopy_uniform_t* uniform, const float* values, uint16_t columns) {
	uint8_t* dst = spoopy_sokol_uniform_dst(uniform);
	if(!dst || !values) {
		return;
	}

	const spoopy_data_type_t* type = &uniform->buffer_backed.data_type;
	if(type->matrix_columns != columns || type->base_type != SPOOPY_SHADER_BASE_TYPE_FP32) {
		return;
	}

	const size_t column_size = sizeof(float) * type->vector_size;
	const size_t stride = type->matrix_stride ? type->matrix_stride : column_size;

	for(uint16_t i = 0; i < columns; ++i) {
		memcpy(dst + i * stride, values + i * type->vector_size, column_size);
	}
}

// TODO (Uniform): Replace this Sokol uniform upload path with the backend-specific
// native upload path once the Metal/WGPU wrapper is in place.
static void spoopy_sokol_apply_uniform_buffers(spoopy_pipeline_t* pipeline) {
	if(pipeline->shader_stages.vertex && pipeline->shader_stages.vertex->uniform_buffer.data && pipeline->shader_stages.vertex->uniform_buffer.size) {
		sg_apply_uniforms(pipeline->shader_stages.vertex->uniform_buffer.slot, &(sg_range) {
			.ptr = pipeline->shader_stages.vertex->uniform_buffer.data,
			.size = pipeline->shader_stages.vertex->uniform_buffer.size,
		});
	}

	if(pipeline->shader_stages.fragment && pipeline->shader_stages.fragment->uniform_buffer.data && pipeline->shader_stages.fragment->uniform_buffer.size) {
		sg_apply_uniforms(pipeline->shader_stages.fragment->uniform_buffer.slot, &(sg_range) {
			.ptr = pipeline->shader_stages.fragment->uniform_buffer.data,
			.size = pipeline->shader_stages.fragment->uniform_buffer.size,
		});
	}
}

static spoopy_uniform_t* spoopy_sokol_shader_uniform(spoopy_pipeline_t* pipeline, const char* name) {
	return spoopy_uniform_ht_get(pipeline ? &pipeline->uniforms : NULL, name);
}

static void spoopy_sokol_uniform_set_int(spoopy_uniform_t* uniform, int value) {
	spoopy_sokol_uniform_write_bytes(uniform, &value, sizeof(value));
}

static void spoopy_sokol_uniform_set_int2(spoopy_uniform_t* uniform, int v0, int v1) {
	const int values[] = { v0, v1 };
	spoopy_sokol_uniform_write_bytes(uniform, values, sizeof(values));
}

static void spoopy_sokol_uniform_set_int3(spoopy_uniform_t* uniform, int v0, int v1, int v2) {
	const int values[] = { v0, v1, v2 };
	spoopy_sokol_uniform_write_bytes(uniform, values, sizeof(values));
}

static void spoopy_sokol_uniform_set_int4(spoopy_uniform_t* uniform, int v0, int v1, int v2, int v3) {
	const int values[] = { v0, v1, v2, v3 };
	spoopy_sokol_uniform_write_bytes(uniform, values, sizeof(values));
}

static void spoopy_sokol_uniform_set_ints(spoopy_uniform_t* uniform, const int* values, int count) {
	if(count > 0) {
		spoopy_sokol_uniform_write_bytes(uniform, values, sizeof(*values) * (size_t)count);
	}
}

static void spoopy_sokol_uniform_set_float(spoopy_uniform_t* uniform, float value) {
	spoopy_sokol_uniform_write_bytes(uniform, &value, sizeof(value));
}

static void spoopy_sokol_uniform_set_float2(spoopy_uniform_t* uniform, float v0, float v1) {
	const float values[] = { v0, v1 };
	spoopy_sokol_uniform_write_bytes(uniform, values, sizeof(values));
}

static void spoopy_sokol_uniform_set_float3(spoopy_uniform_t* uniform, float v0, float v1, float v2) {
	const float values[] = { v0, v1, v2 };
	spoopy_sokol_uniform_write_bytes(uniform, values, sizeof(values));
}

static void spoopy_sokol_uniform_set_float4(spoopy_uniform_t* uniform, float v0, float v1, float v2, float v3) {
	const float values[] = { v0, v1, v2, v3 };
	spoopy_sokol_uniform_write_bytes(uniform, values, sizeof(values));
}

static void spoopy_sokol_uniform_set_floats(spoopy_uniform_t* uniform, const float* values, int count) {
	if(count > 0) {
		spoopy_sokol_uniform_write_bytes(uniform, values, sizeof(*values) * (size_t)count);
	}
}

static void spoopy_sokol_uniform_set_bool(spoopy_uniform_t* uniform, bool value) {
	const int32_t storage = value ? 1 : 0;
	spoopy_sokol_uniform_write_bytes(uniform, &storage, sizeof(storage));
}

static void spoopy_sokol_uniform_set_matrix3(spoopy_uniform_t* uniform, const float* values) {
	spoopy_sokol_uniform_write_matrix(uniform, values, 3);
}

static void spoopy_sokol_uniform_set_matrix4(spoopy_uniform_t* uniform, const float* values) {
	spoopy_sokol_uniform_write_matrix(uniform, values, 4);
}

static void spoopy_sokol_pipeline_bind(spoopy_pipeline_t* pipeline) {
	assert(pipeline);
	sg_apply_pipeline(pipeline->pipeline);
}

static void spoopy_sokol_draw_mesh(const spoopy_mesh_t* mesh, spoopy_pipeline_t* pipeline) {
	assert(mesh);
	assert(pipeline);
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
	spoopy_sokol_apply_uniform_buffers(pipeline);

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
	spoopy_uniform_t* uniform = spoopy_sokol_shader_uniform(pipeline, name);
	if(!uniform || !spoopy_sokol_uniform_is_sampler(uniform->type)) {
		return 0;
	}

	return (uint8_t)uniform->sampler.binding;
}

void spoopy_sokol_texture_set(spoopy_pipeline_t* pipeline, const char* uniform_name, spoopy_texture_t* tex) {
	if(!pipeline) {
		SPOOPY_LOG_WARN("Texture bind request ignored: pipeline is NULL");
		return;
	}

	spoopy_uniform_t* uniform = spoopy_sokol_shader_uniform(pipeline, uniform_name);
	if(!uniform || !spoopy_sokol_uniform_is_sampler(uniform->type)) {
		SPOOPY_LOG_WARN("Texture bind request ignored: '%s' is not a sampler uniform", uniform_name ? uniform_name : "(null)");
		return;
	}

	const uint16_t tex_slot = uniform->sampler.binding;
	const uint16_t sampler_slot = uniform->sampler.paired_binding;
	if(tex_slot >= SG_MAX_VIEW_BINDSLOTS || sampler_slot >= SG_MAX_SAMPLER_BINDSLOTS || sampler_slot == UINT16_MAX) {
		SPOOPY_LOG_WARN("Texture bind request out of range: view=%u sampler=%u", tex_slot, sampler_slot);
		return;
	}

	if(!tex) {
		pipeline->bindings.views[tex_slot] = (sg_view){0};
		pipeline->bindings.samplers[sampler_slot] = (sg_sampler){0};
		return;
	}

	if(!spoopy_sokol_texture_set_binding_indices(tex, tex_slot, sampler_slot)) {
		return;
	}

	// Temporary debug logging for runtime texture binding.
	SPOOPY_LOG_INFO(
		"Texture bind: uniform='%s' texture_slot=%u sampler_slot=%u image=%u view=%u sampler=%u format=%d stage=%d",
		uniform_name ? uniform_name : "(null)",
		tex_slot,
		sampler_slot,
		tex->image.id,
		tex->view.id,
		tex->sampler_state.id,
		tex->params.format,
		tex->params.stage
	);

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
	.shader_uniform = spoopy_sokol_shader_uniform,
	.uniform_set_int = spoopy_sokol_uniform_set_int,
	.uniform_set_int2 = spoopy_sokol_uniform_set_int2,
	.uniform_set_int3 = spoopy_sokol_uniform_set_int3,
	.uniform_set_int4 = spoopy_sokol_uniform_set_int4,
	.uniform_set_ints = spoopy_sokol_uniform_set_ints,
	.uniform_set_float = spoopy_sokol_uniform_set_float,
	.uniform_set_float2 = spoopy_sokol_uniform_set_float2,
	.uniform_set_float3 = spoopy_sokol_uniform_set_float3,
	.uniform_set_float4 = spoopy_sokol_uniform_set_float4,
	.uniform_set_floats = spoopy_sokol_uniform_set_floats,
	.uniform_set_bool = spoopy_sokol_uniform_set_bool,
	.uniform_set_matrix3 = spoopy_sokol_uniform_set_matrix3,
	.uniform_set_matrix4 = spoopy_sokol_uniform_set_matrix4,
	.blend = spoopy_sokol_blend,
	.blend_current = spoopy_sokol_blend_current,
	.cull = spoopy_sokol_cull,
	.cull_current = spoopy_sokol_cull_current,
	.capabilities = spoopy_sokol_capabilities,
	.capabilities_current = spoopy_sokol_capabilities_current,
};
