#include <spoopy_api.h>

#include "test_renderer.h"

typedef struct vertex2d {
	float pos[2];
	float uv[2];
	float color[4];
} vertex2d_t;

int main(int argc, char** argv) {
	test_init();

	const char* shader_vert =
		"#ifndef ShaderTypes_h\n"
		"#define ShaderTypes_h\n"
		"#endif\n"
		"\n"
		"struct VertexInput\n"
		"{\n"
		"    float2 pos : POSITION;\n"
		"    float2 uv : TEXCOORD0;\n"
		"    float4 color : COLOR0;\n"
		"};\n"
		"\n"
		"struct VertexOutput\n"
		"{\n"
		"    float4 v_position : SV_POSITION;\n"
		"    float2 v_uv : TEXCOORD0;\n"
		"    float4 v_color : COLOR0;\n"
		"};\n"
		"\n"
		"[shader(\"vertex\")]\n"
		"VertexOutput vertexMain(VertexInput input)\n"
		"{\n"
		"    VertexOutput output;\n"
		"    output.v_position = float4(input.pos.x, input.pos.y, 0.0, 1.0);\n"
		"    output.v_uv = input.uv;\n"
		"    output.v_color = input.color;\n"
		"    return output;\n"
		"}\n";

	const char* shader_frag =
		"#ifndef ShaderTypes_h\n"
		"#define ShaderTypes_h\n"
		"#endif\n"
		"\n"
		"struct VertexOutput {\n"
		"    float4 v_position : SV_POSITION;\n"
		"    float2 v_uv : TEXCOORD0;\n"
		"    float4 v_color : COLOR0;\n"
		"};\n"
		"\n"
		"Texture2D tex0;\n"
		"SamplerState samp0;\n"
		"\n"
		"[shader(\"fragment\")]\n"
		"float4 fragmentMain(VertexOutput input) : SV_Target {\n"
		"    return tex0.Sample(samp0, input.v_uv) * input.v_color;\n"
		"}\n";

	spoopy_shader_object_t* vert_obj = load_shader(shader_vert, SPOOPY_STAGE_VERTEX);
	spoopy_shader_object_t* frag_obj = load_shader(shader_frag, SPOOPY_STAGE_FRAGMENT);
    spoopy_pipeline_t* pipeline = spoopy_api_pipeline_link(2, (spoopy_shader_object_t*[]){ vert_obj, frag_obj });

	spoopy_vertex_attr_spec_t vertex_spec[] = {
		{ 2, SPOOPY_VA_FLOAT, SPOOPY_VA_CONV_FLOAT },
		{ 2, SPOOPY_VA_FLOAT, SPOOPY_VA_CONV_FLOAT },
		{ 4, SPOOPY_VA_FLOAT, SPOOPY_VA_CONV_FLOAT }
	};

	spoopy_api_pipeline_compile(pipeline, 3, vertex_spec, 0);
	spoopy_api_shader_destroy(vert_obj, true);
	spoopy_api_shader_destroy(frag_obj, true);

	struct vertex2d vertices[] = {
		{ {  1, -1, }, { 1, 0 }, { 1, 1, 1, 1 }, },
		{ {  1,  1, }, { 1, 1 }, { 1, 1, 1, 1 }, },
		{ { -1, -1, }, { 0, 0 }, { 1, 1, 1, 1 }, },
		{ { -1,  1, }, { 0, 1 }, { 1, 1, 1, 1 }, },
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

	spoopy_texture_t* tex = test_renderer_load_texture("test/tung.png");
	spoopy_api_texture_set(pipeline, "tex0", "samp0", tex);
	while(!spoopy_api_should_quit()) {
		spoopy_events_poll(handler_ptr, 0);
		spoopy_api_clear(SPOOPY_BUFFER_ALL, SPOOPY_RGB(0.0, 0.0, 0.0), 0.0f);
		spoopy_api_pipeline_bind(pipeline);
		spoopy_api_draw_mesh(&mesh, pipeline);
		spoopy_api_swap_buffers();
	}

	spoopy_api_texture_destroy(tex);
	spoopy_heap_free(tex);
	spoopy_api_video_shutdown();
	return 0;
}
