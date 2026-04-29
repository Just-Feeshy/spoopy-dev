#include <spoopy_api.h>
#include <SDL3/SDL_iostream.h>

#if SPOOPY_FUKKIRETA_FASTNOISELITE
    #define FNL_IMPL
    #include <FastNoiseLite.h>
#endif

typedef struct vertex2d {
	float pos[2];
	float resolution[2];
} vertex2d_t;

static spoopy_event_handler_t* handler_ptr = NULL;

#if SPOOPY_FUKKIRETA_FASTNOISELITE
static const int fukkireta_fastnoiselite_seed = 1337;
static const float fukkireta_fastnoiselite_frequency = 0.060f;
#endif

static spoopy_texture_t* create_noise_texture(uint32_t width, uint32_t height) {
#if !SPOOPY_FUKKIRETA_FASTNOISELITE
	(void)width;
	(void)height;
	SPOOPY_LOG_ERROR("fukkireta noise texture requested, but FastNoiseLite is disabled. Reconfigure with -Dfastnoiselite=true.");
	return NULL;
#else
	const size_t pixel_count = (size_t)width * (size_t)height;
	const size_t data_size = pixel_count * 4;
	uint8_t* pixels = spoopy_heap_alloc(data_size);

	if(!pixels) {
		SPOOPY_LOG_ERROR("Failed to allocate noise texture pixels");
		return NULL;
	}

	fnl_state noise = fnlCreateState();
	noise.seed = fukkireta_fastnoiselite_seed;
	noise.frequency = fukkireta_fastnoiselite_frequency;

	for(uint32_t y = 0; y < height; ++y) {
		for(uint32_t x = 0; x < width; ++x) {
			const size_t index = ((size_t)y * (size_t)width + (size_t)x) * 4;
			const float sample = fnlGetNoise2D(&noise, (float)x, (float)y);
			const uint8_t shade = (uint8_t)((sample * 0.5f + 0.5f) * 255.0f);

			pixels[index + 0] = shade;
			pixels[index + 1] = shade;
			pixels[index + 2] = shade;
			pixels[index + 3] = 255;
		}
	}

	spoopy_image_t image = {
		.pixels.raw_data = pixels,
		.width = width,
		.height = height,
		.data_size = (uint32_t)data_size,
		.format = SPOOPY_PIXEL_FORMAT_RGBA8,
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
			.min = SPOOPY_TEXTURE_FILTER_LINEAR,
			.mag = SPOOPY_TEXTURE_FILTER_LINEAR
		},
		.wrap = {
			.u = SPOOPY_TEXTURE_WRAP_CLAMP,
			.v = SPOOPY_TEXTURE_WRAP_CLAMP
		}
	};

	spoopy_api_texture_create(tex, &params);
	spoopy_api_texture_fill(tex, 0, 0, &image);
	spoopy_heap_free(pixels);
	return tex;
#endif
}

static char* load_shader(const char* path) {

	// Read the Slang source from disk so this test covers file-backed shader loading.
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

	// Resolve the active backend target before asking Slang to emit backend code.
	if(!spoopy_api_shader_supported(&source, &transpile_opts)) {
		SPOOPY_LOG_ERROR("Shader is not supported: %s", path);
		spoopy_heap_free(src);
		return NULL;
	}

	spoopy_shader_source_t transpiled = {0};
	spoopy_mem_arena_t transpile_arena = {0};
	spoopy_arena_init(&transpile_arena, source.content_size + (1 << 11));

	// Transpile once into runtime shader code, then materialize the backend object immediately.
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
	spoopy_texture_t* noise_texture = create_noise_texture(256, 256);

	if(!noise_texture) {
		return 1;
	}

	spoopy_vertex_attr_spec_t vertex_spec[] = {
		{ 2, SPOOPY_VA_FLOAT, SPOOPY_VA_CONV_FLOAT },
		{ 2, SPOOPY_VA_FLOAT, SPOOPY_VA_CONV_FLOAT }
	};

	spoopy_api_pipeline_compile(pipeline, 2, vertex_spec, 0);
	spoopy_api_texture_set(pipeline, "tex0", noise_texture);

	// Fullscreen clip-space quad.
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

	while(!spoopy_api_should_quit()) {

		// Keep the render path intentionally simple: clear, bind, draw, present.
		spoopy_events_poll(handler_ptr, 0);
		spoopy_api_clear(SPOOPY_BUFFER_COLOR, SPOOPY_RGB(0.0, 0.0, 0.0), 0.0f);
		spoopy_api_pipeline_bind(pipeline);
		spoopy_api_draw_mesh(&mesh, pipeline);
		spoopy_api_swap_buffers();
	}

	spoopy_api_texture_destroy(noise_texture);
	spoopy_heap_free(noise_texture);
	spoopy_heap_free(handler_ptr);
	spoopy_api_video_shutdown();
	return 0;
}
