#pragma once

#include <SDL3/SDL_iostream.h>

#include <spoopy_image.h>

#include <spoopy_api.h>

static spoopy_event_handler_t* handler_ptr = NULL;

SPOOPY_ATTR_UNUSED static const spoopy_vec2_int_t VIEWPORT = (spoopy_vec2_int_t) {
	.w = 800,
	.h = 600,
};

SPOOPY_ATTR_UNUSED static const char* test_renderer_name(spoopy_renderer_t renderer) {
	switch(renderer) {
		case SPOOPY_RENDERER_API_METAL: return "Metal";
		case SPOOPY_RENDERER_API_WGPU:  return "WGPU";
		case SPOOPY_RENDERER_API_UNSURE: return "Unknown";
		default: return "Unknown";
	}
}

SPOOPY_ATTR_UNUSED static void test_log_shader_support_failure(
	const spoopy_shader_source_t* source,
	const spoopy_transpile_options_t* transpile_opts
) {
	if(!source || !transpile_opts) {
		SPOOPY_LOG_ERROR("Shader support check failed with invalid parameters.");
		return;
	}

	if(transpile_opts->compile.renderer != SPOOPY_RENDERER_API_UNSURE) {
		SPOOPY_LOG_ERROR(
			"%s renderer is available, but Slang has no compatible compile target/profile.",
			test_renderer_name(transpile_opts->compile.renderer)
		);
		return;
	}

	if(source->target == SPOOPY_RENDERER_API_UNSURE) {
		SPOOPY_LOG_ERROR("No renderer selected for shader target.");
		return;
	}

	SPOOPY_LOG_ERROR(
		"%s renderer is not supported on this platform.",
		test_renderer_name(source->target)
	);
}

SPOOPY_ATTR_UNUSED static void test_init(void) {
    SPOOPY_LOG_INFO("Test Renderer Initialized");

	spoopy_memory_init_hooks();
	spoopy_events_init(0, &handler_ptr);

#ifndef __EMSCRIPTEN__
    spoopy_sys_thread_init();
#endif

    spoopy_api_video_init(&(spoopy_video_init_params_t) {
        .title = "Test Renderer",
        .width = VIEWPORT.w,
        .height = VIEWPORT.h,
		.renderer = SPOOPY_RENDERER_API_BEST_OPTION,
    });
}

SPOOPY_ATTR_UNUSED static spoopy_shader_object_t* load_shader(const char* src, spoopy_shader_stage_t stage) {
    spoopy_shader_source_t source = {
        .content = src,
        .content_size = strlen(src),
        .stage = stage,
        .entry_point = (stage == SPOOPY_STAGE_VERTEX) ? "vertexMain" : "fragmentMain",
        .module_name = "shader",
		.target = spoopy_api_window_get_renderer(),
    };

    spoopy_transpile_options_t transpile_opts = {
        .filename = "<embedded>",
		.compile.optimization_level = SPOOPY_OPTIMIZATION_LEVEL_NONE,
    };

    if(!spoopy_api_shader_supported(&source, &transpile_opts)) {
		test_log_shader_support_failure(&source, &transpile_opts);
        return NULL;
    }

    spoopy_shader_source_t new_src = {0};
	spoopy_mem_arena_t transpile_arena = {0};
	spoopy_arena_init(&transpile_arena, source.content_size + (1 << 11));
    bool result = spoopy_api_shader_transpile(
        &source,
        &new_src,
        &transpile_opts,
		&transpile_arena
    );

    if(!result) {
        SPOOPY_LOG_ERROR("Failed to transpile shader: %s", source.entry_point);
		spoopy_arena_deinit(&transpile_arena);
        return NULL;
    }

	SPOOPY_LOG_SUCCESS("Shader transpiled successfully: %s", source.entry_point);
	spoopy_shader_object_t* shader = spoopy_heap_alloc(spoopy_shader_object_size);

	if(!shader) {
		SPOOPY_LOG_ERROR("Failed to compile shader!");
		spoopy_arena_deinit(&transpile_arena);
		return NULL;
	}

	bool init_ok = spoopy_api_shader_init(shader, &new_src);
	spoopy_arena_deinit(&transpile_arena);

	if(!init_ok) {
		SPOOPY_LOG_ERROR("Failed to initialize shader object: %s", source.entry_point);
		spoopy_api_shader_destroy(shader, true);
		return NULL;
	}

    SPOOPY_LOG_SUCCESS("Shader compiled and ready for use: %s", source.entry_point);
    return shader;
}

SPOOPY_ATTR_UNUSED static spoopy_texture_t* test_renderer_load_texture(const char* path) {
    SDL_IOStream* io = SDL_IOFromFile(path, "rb");

    if(!io) {
		SPOOPY_LOG_ERROR("Failed to open texture file: %s", path);
		return NULL;
    }

    spoopy_image_t image = {0};

    if(!spoopy_image_load_stream(io, SPOOPY_IMAGE_FILE_FORMAT_AUTO, &image)) {
		SPOOPY_LOG_ERROR("Failed to load image from file: %s", path);
		SDL_CloseIO(io);
		return NULL;
    }

    SDL_CloseIO(io);

    spoopy_texture_t* tex = spoopy_heap_alloc(spoopy_api_texture_size());
    if(!tex) {
		SPOOPY_LOG_ERROR("Failed to allocate texture object for %s", path);
		spoopy_heap_free(image.pixels.raw_data);
		return NULL;
    }

    const spoopy_texture_params_t params = {
		.width = image.width,
		.height = image.height,
		.layers = 1,
		.mipmaps = 1,
		.format = image.format,
		.texture_class = SPOOPY_TEXTURE_CLASS_2D,
		.stage = SPOOPY_STAGE_FRAGMENT,
		.depth_texture = false,
		.filter = {
			.min = SPOOPY_TEXTURE_FILTER_LINEAR,
			.mag = SPOOPY_TEXTURE_FILTER_LINEAR
		},
		.wrap = {
			.u = SPOOPY_TEXTURE_WRAP_REPEAT,
			.v = SPOOPY_TEXTURE_WRAP_REPEAT
		}
    };

    spoopy_api_texture_create(tex, &params);
    spoopy_api_texture_fill(tex, 0, 0, &image);
    spoopy_heap_free(image.pixels.raw_data);

    return tex;
}
