#pragma once

#include <spoopy.h>
#include <spoopy_log.h>
#include <spoopy_graphics.h>
#include <spoopy_shader.h>
#include <memory/spoopy_memory.h>

#if defined(SPOOPY_SOKOL_IMPLEMENTATION)
#define SOKOL_IMPL
#elif defined(SOKOL_IMPL)
#error "Define SPOOPY_SOKOL_IMPLEMENTATION in exactly one translation unit instead of SOKOL_IMPL."
#endif

#if (defined(SPOOPY_RENDERER_METAL) + defined(SPOOPY_RENDERER_D3D11) + defined(SPOOPY_RENDERER_WGPU)) == 0
#error "No SPOOPY_RENDERER_* backend defined for Sokol."
#endif

#if defined(SPOOPY_RENDERER_METAL)
#define SOKOL_METAL
#endif
#if defined(SPOOPY_RENDERER_D3D11)
#define SOKOL_D3D11
#endif
#if defined(SPOOPY_RENDERER_WGPU)
#define SOKOL_WGPU
#endif

#include <sokol_gfx.h>

static inline void* spoopy_sokol_alloc(size_t size, void* user_data) {
	return spoopy_aligned_alloc(SPOOPY_MAX_ALIGN, size, user_data);
}

#define SPOOPY_SG_DEFAULT_DESC \
	.allocator = { \
		.alloc_fn = spoopy_sokol_alloc, \
		.free_fn = spoopy_sokol_free, \
		.user_data = NULL, \
	}, \
	.logger = { \
		.func = spoopy_sokol_log_cb, \
	},

static inline void spoopy_sokol_free(void* ptr, void* user_data) {
	(void)user_data;
	spoopy_heap_free(ptr);
}

static inline void spoopy_sokol_log_cb(
    const char* tag,
    uint32_t log_level,
    uint32_t log_item_id,
    const char* message_or_null,
    uint32_t line_nr,
    const char* filename_or_null,
    void* user_data)
{
    (void)tag; (void)log_item_id; (void)line_nr; (void)filename_or_null; (void)user_data;

    const char* msg = message_or_null ? message_or_null : "(null)";

    // log_level meanings are “severity-ish”. Exact numeric values aren’t important;
    // map by range / expectation.
    switch (log_level) {
        default:
        case 0: SPOOPY_LOG_INFO("%s", msg); break;
        case 1: SPOOPY_LOG_WARN("%s", msg); break;
        case 2: SPOOPY_LOG_ERROR("%s", msg); break;
    }
}

#ifdef __cplusplus
extern "C" {
#endif

extern sg_swapchain spoopy_swapchain;

struct spoopy_vertex_buffer {
	sg_buffer buffer;
	uint32_t stride;
	uint32_t count;
};

struct spoopy_index_buffer {
	sg_buffer buffer;
	uint32_t count;
};

struct spoopy_pipeline {
	sg_shader shader;
	sg_pipeline pipeline;
	sg_bindings bindings;
};

struct spoopy_shader_object {
	spoopy_shader_stage_t stage;
	sg_shader_function func;
	bool owns_source;
};

void spoopy_sokol_init(void);

#ifdef __cplusplus
}
#endif
