#define SPOOPY_BUILD_DEBUG
#include <spoopy_api.h>

#include "test_renderer.h"

#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include <cimgui.h>
#include "ui_impl.h"

typedef struct imgui_packed_vertex_t {
    uint32_t packed0;
    uint32_t packed1;
} imgui_packed_vertex_t;

static uint32_t q12(float x) {
    if (x < 0.0f) x = 0.0f;
    if (x > 1.0f) x = 1.0f;
    return (uint32_t)(x * 4095.0f + 0.5f);
}

static uint32_t q4(float x) {
    if (x < 0.0f) x = 0.0f;
    if (x > 1.0f) x = 1.0f;
    return (uint32_t)(x * 15.0f + 0.5f);
}

static imgui_packed_vertex_t pack_vertex(
    float pos_x01,
    float pos_y01,
    float uv_x,
    float uv_y,
    float r,
    float g,
    float b,
    float a
) {
    uint32_t xq = q12(pos_x01);
    uint32_t yq = q12(pos_y01);
    uint32_t uq = q12(uv_x);
    uint32_t vq = q12(uv_y);

    uint32_t rq = q4(r);
    uint32_t gq = q4(g);
    uint32_t bq = q4(b);
    uint32_t aq = q4(a);

    imgui_packed_vertex_t out;

    out.packed0 =
        xq |
        (yq << 12) |
        ((uq & 0xFFu) << 24);

    out.packed1 =
        ((uq >> 8) & 0xFu) |
        (vq << 4) |
        (rq << 16) |
        (gq << 20) |
        (bq << 24) |
        (aq << 28);

    return out;
}

static char* load_shader_file(const char* path) {
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

int main(int argc, char** argv) {
	VIEWPORT = (spoopy_vec2_int_t) {
		.w = 1280,
		.h = 720,
	};

	test_init();

	igCreateContext(NULL);
	if(!spoopy_ui_impl_init()) {
		SPOOPY_LOG_ERROR("Failed to initialize ImGui UI backend");
		return 1;
	}

	ImGuiIO io = *igGetIO_Nil(); (void)io;
	igStyleColorsDark(NULL);

	char* shader_file = load_shader_file("test/ui/vertex.slang");
	spoopy_shader_object_t* vert_obj = load_shader(shader_file, SPOOPY_STAGE_VERTEX);
	spoopy_heap_free(shader_file);

	shader_file = load_shader_file("test/ui/fragment.slang");
	spoopy_shader_object_t* frag_obj = load_shader(shader_file, SPOOPY_STAGE_FRAGMENT);
	spoopy_heap_free(shader_file);

	spoopy_pipeline_t* pipeline = spoopy_api_pipeline_link(2, (spoopy_shader_object_t*[]){ vert_obj, frag_obj });

	spoopy_vertex_attr_spec_t vertex_spec[] = {
		{ 2, SPOOPY_VA_UINT, SPOOPY_VA_CONV_INT }
	};

	spoopy_api_pipeline_compile(pipeline, 1, vertex_spec, 0);

	imgui_packed_vertex_t vertices[] = {
		pack_vertex(1.0f, 1.0f, 1.0f, 1.0f, 1, 1, 1, 1),
		pack_vertex(1.0f, 0.0f, 1.0f, 0.0f, 1, 1, 1, 1),
		pack_vertex(0.0f, 1.0f, 0.0f, 1.0f, 1, 1, 1, 1),
		pack_vertex(0.0f, 0.0f, 0.0f, 0.0f, 1, 1, 1, 1),
	};

	uint16_t indices[] = { 0, 1, 2, 2, 1, 3 };

	spoopy_vertex_buffer_t* vbuf = spoopy_stack_alloc(spoopy_api_buffer_size(SPOOPY_BUFFER_TYPE_VERTEX));
	spoopy_index_buffer_t* ibuf = spoopy_stack_alloc(spoopy_api_buffer_size(SPOOPY_BUFFER_TYPE_INDEX));

	if(!spoopy_api_vertex_buffer_create(vbuf, (uint32_t)sizeof(vertices), 4, vertices, 0)) {
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
		spoopy_events_poll(handler_ptr, 0);
		spoopy_api_clear(SPOOPY_BUFFER_COLOR, SPOOPY_RGB(0.0, 0.0, 0.0), 0.0f);
		spoopy_api_pipeline_bind(pipeline);
		spoopy_api_draw_mesh(&mesh, pipeline);
		spoopy_api_swap_buffers();
	}

	spoopy_ui_impl_shutdown();
	spoopy_api_video_shutdown();
	return 0;
}
