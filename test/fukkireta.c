#include <spoopy_api.h>
#include <SDL3/SDL_iostream.h>

typedef struct vertex2d {
	float pos[2];
	float resolution[2];
} vertex2d_t;

static spoopy_event_handler_t* handler_ptr = NULL;

// static const int fukkireta_noise_seed = 1337;

#if 0
// Temporary: texture experiment disabled while isolating fragment performance.
static uint32_t fukkireta_noise_hash(uint32_t x, uint32_t y, uint32_t seed) {
	uint32_t v = seed ^ (x * 0x9E3779B9u) ^ (y * 0x85EBCA6Bu);
	v ^= v >> 16;
	v *= 0x7FEB352Du;
	v ^= v >> 15;
	v *= 0x846CA68Bu;
	v ^= v >> 16;
	return v;
}

static spoopy_texture_t* create_noise_texture(uint32_t width, uint32_t height) {
	const size_t pixel_count = (size_t)width * (size_t)height;
	const size_t data_size = pixel_count;
	uint8_t* pixels = spoopy_heap_alloc(data_size);

	if(!pixels) {
		SPOOPY_LOG_ERROR("Failed to allocate noise texture pixels");
		return NULL;
	}

	for(uint32_t y = 0; y < height; ++y) {
		for(uint32_t x = 0; x < width; ++x) {
			const size_t index = (size_t)y * (size_t)width + (size_t)x;
			pixels[index] = (uint8_t)(fukkireta_noise_hash(x, y, (uint32_t)fukkireta_noise_seed) & 0xFFu);
		}
	}

	spoopy_image_t image = {
		.pixels.raw_data = pixels,
		.width = width,
		.height = height,
		.data_size = (uint32_t)data_size,
		.format = SPOOPY_PIXEL_FORMAT_R8,
		.origin = SPOOPY_IMAGE_ORIGIN_TOP_LEFT,
	};

	spoopy_texture_t* tex = spoopy_heap_alloc(spoopy_api_texture_size());
	if(!tex) {
		SPOOPY_LOG_ERROR("Failed to allocate noise texture object");
		spoopy_heap_free(pixels);
		return NULL;
	}

	const spoopy_texture_params_t params = {
		.width = width,
		.height = height,
		.layers = 1,
		.mipmaps = 1,
		.format = image.format,
		.texture_class = SPOOPY_TEXTURE_CLASS_2D,
		.stage = SPOOPY_STAGE_FRAGMENT,
		.depth_texture = false,
		.filter = {
			.min = SPOOPY_TEXTURE_FILTER_NEAREST,
			.mag = SPOOPY_TEXTURE_FILTER_NEAREST
		},
		.wrap = {
			.u = SPOOPY_TEXTURE_WRAP_REPEAT,
			.v = SPOOPY_TEXTURE_WRAP_REPEAT
		}
	};

	spoopy_api_texture_create(tex, &params);
	spoopy_api_texture_fill(tex, 0, 0, &image);
	spoopy_heap_free(pixels);
	return tex;
}
#endif

static char* load_shader(const char* path) {

	SDL_IOStream* io = SDL_IOFromFile(path, "rb");

	if(!io) {
		SPOOPY_LOG_ERROR("Failed to open shader file: %s", path);
		return NULL;
	}

	const Sint64 size = SDL_GetIOSize(io);
	if(size <= 0) {
		SPOOPY_LOG_ERROR("Failed to read shader file size: %s", path);
		SDL_CloseIO(io);
		return NULL;
	}

	char* source = spoopy_heap_alloc((size_t)size + 1);
	if(!source) {
		SDL_CloseIO(io);
		return NULL;
	}

	const size_t bytes_read = SDL_ReadIO(io, source, (size_t)size);
	SDL_CloseIO(io);

	if(bytes_read != (size_t)size) {
		SPOOPY_LOG_ERROR("Failed to read shader file contents: %s", path);
		spoopy_heap_free(source);
		return NULL;
	}

	source[size] = '\0';
	return source;
}

static spoopy_shader_object_t* load_shader_object(const char* path, spoopy_shader_stage_t stage) {
	char* src = load_shader(path);
	if(!src) {
		return NULL;
	}

	spoopy_shader_source_t source = {
		.content = src,
		.content_size = strlen(src),
		.stage = stage,
		.entry_point = (stage == SPOOPY_STAGE_VERTEX) ? "vertexMain" : "fragmentMain",
		.module_name = "fukkireta",
		.target = spoopy_api_window_get_renderer(),
	};

	spoopy_transpile_options_t transpile_opts = {
		.filename = path,
		.compile.optimization_level = SPOOPY_OPTIMIZATION_LEVEL_NONE,
	};

	if(!spoopy_api_shader_supported(&source, &transpile_opts)) {
		SPOOPY_LOG_ERROR("Shader is not supported: %s", path);
		spoopy_heap_free(src);
		return NULL;
	}

	spoopy_shader_source_t transpiled = {0};
	spoopy_mem_arena_t transpile_arena = {0};
	spoopy_arena_init(&transpile_arena, source.content_size + (1 << 11));

	if(!spoopy_api_shader_transpile(&source, &transpiled, &transpile_opts, &transpile_arena)) {
		SPOOPY_LOG_ERROR("Failed to transpile shader: %s", path);
		spoopy_arena_deinit(&transpile_arena);
		spoopy_heap_free(src);
		return NULL;
	}

	spoopy_shader_object_t* shader = spoopy_heap_alloc(spoopy_shader_object_size);
	if(!shader) {
		spoopy_arena_deinit(&transpile_arena);
		spoopy_heap_free(src);
		return NULL;
	}

	if(!spoopy_api_shader_init(shader, &transpiled)) {
		SPOOPY_LOG_ERROR("Failed to initialize shader object: %s", path);
		spoopy_api_shader_destroy(shader, true);
		spoopy_arena_deinit(&transpile_arena);
		spoopy_heap_free(src);
		return NULL;
	}

	spoopy_arena_deinit(&transpile_arena);
	spoopy_heap_free(src);
	return shader;
}

int	main(void) {
	const float window_width = 1280.0f;
	const float window_height = 720.0f;

	spoopy_memory_init_hooks();
	spoopy_events_init(0, &handler_ptr);

	spoopy_api_video_init(&(spoopy_video_init_params_t) {
		.title = "Fukkireta",
		.width = (uint32_t)window_width,
		.height = (uint32_t)window_height,
		.renderer = SPOOPY_RENDERER_API_BEST_OPTION,
	});

	spoopy_shader_object_t* vert_obj = load_shader_object("test/fukkireta/vertex.slang", SPOOPY_STAGE_VERTEX);
	spoopy_shader_object_t* frag_obj = load_shader_object("test/fukkireta/fragment.slang", SPOOPY_STAGE_FRAGMENT);
	spoopy_pipeline_t* pipeline = spoopy_api_pipeline_link(2, (spoopy_shader_object_t*[]){ vert_obj, frag_obj });
	/* Temporary: texture experiment disabled.
	spoopy_texture_t* noise_texture = create_noise_texture(64, 64);

	if(!noise_texture) {
		return 1;
	}
	*/

	spoopy_fps_counter_t* fps = SPOOPY_INIT_FPS_COUNTER(120, spoopy_stack);

	spoopy_vertex_attr_spec_t vertex_spec[] = {
		{ 2, SPOOPY_VA_FLOAT, SPOOPY_VA_CONV_FLOAT },
		{ 2, SPOOPY_VA_FLOAT, SPOOPY_VA_CONV_FLOAT }
	};

	spoopy_api_pipeline_compile(pipeline, 2, vertex_spec, 0);
	/* Temporary: texture experiment disabled.
	spoopy_api_texture_set(pipeline, "tex0", noise_texture);
	*/

	// Fullscreen clip-space quad.
	// TODO (Swapchain): Have resolution be a uniform and be iResolution like shadertoy
	vertex2d_t vertices[] = {
		{ { -1.0f, -1.0f }, { window_width, window_height } },
		{ {  1.0f, -1.0f }, { window_width, window_height } },
		{ {  1.0f,  1.0f }, { window_width, window_height } },
		{ { -1.0f,  1.0f }, { window_width, window_height } },
	};

	uint16_t indices[] = { 0, 1, 2, 0, 2, 3 };

	spoopy_vertex_buffer_t* vbuf = spoopy_stack_alloc(spoopy_api_buffer_size(SPOOPY_BUFFER_TYPE_VERTEX));
	spoopy_index_buffer_t* ibuf = spoopy_stack_alloc(spoopy_api_buffer_size(SPOOPY_BUFFER_TYPE_INDEX));

	if(!spoopy_api_vertex_buffer_create(vbuf, sizeof(vertices), 4, vertices, 0)) {
		SPOOPY_LOG_ERROR("Failed to create vertex buffer");
		return 1;
	}

	if(!spoopy_api_index_buffer_create(ibuf, 6, indices)) {
		SPOOPY_LOG_ERROR("Failed to create index buffer");
		return 1;
	}

	spoopy_mesh_t mesh = {
		.vertex_buffers = vbuf,
		.index_buffer = ibuf,
		.index_count = 6,
		.vertex_count = 1
	};

	size_t last_time = spoopy_time_get();
	size_t last_print_time = last_time;

	while(!spoopy_api_should_quit()) {
		size_t t = spoopy_time_get();

		// Keep the render path intentionally simple: clear, bind, draw, present.
		spoopy_events_poll(handler_ptr, 0);
		spoopy_api_clear(SPOOPY_BUFFER_COLOR, SPOOPY_RGB(0.0, 0.0, 0.0), 0.0f);
		spoopy_api_pipeline_bind(pipeline);
		spoopy_api_draw_mesh(&mesh, pipeline);
		spoopy_api_swap_buffers();

		spoopy_fps_counter_update(fps);

		if(t - last_print_time > SPOOPY_TIME_RESOLUTION) {
			last_print_time = t;
			SPOOPY_LOG_INFO("%.02f FPS", fps->fps);
		}

		last_time = t;
	}

	/* Temporary: texture experiment disabled.
	spoopy_api_texture_destroy(noise_texture);
	spoopy_heap_free(noise_texture);
	*/
	spoopy_heap_free(handler_ptr);
	spoopy_api_video_shutdown();
	return 0;
}
