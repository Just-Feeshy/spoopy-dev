#include <spoopy_api.h>

#include <SDL3/SDL_iostream.h>

#include "test_renderer.h"

typedef struct vertex2d {
	float pos[2];
	float uv[2];
	float color[4];
} vertex2d_t;

int main(int argc, char** argv) {
	test_init();

	const char* shader_vert = R"(
		#ifdef ShaderTypes_h
		#define ShaderTypes_h
		#endif

		struct VertexInput
		{
			float2 pos : POSITION;
			float2 uv : TEXCOORD0;
			float4 color : COLOR0;
		};

		struct VertexOutput
		{
			float4 v_position : SV_POSITION;
			float2 v_uv : TEXCOORD0;
			float4 v_color : COLOR0;
		};

		[shader("vertex")]
		VertexOutput vertexMain(VertexInput input)
		{
			VertexOutput output;
			output.v_position = float4(input.pos.x, input.pos.y, 0.0, 1.0);
			output.v_uv = input.uv;
			output.v_color = input.color;

			return output;
		}
	)";

	const char* shader_frag = R"(
		#ifdef ShaderTypes_h
		#define ShaderTypes_h
		#endif

		struct VertexOutput {
			float4 v_position : SV_POSITION;
			float2 v_uv : TEXCOORD0;
			float4 v_color : COLOR0;
		};

		Texture2D tex0 : register(t0);
		SamplerState samp0 : register(s0);

		[shader("fragment")]
		float4 fragmentMain(VertexOutput input) : SV_Target {
			return tex0.Sample(samp0, input.v_uv) * input.v_color;
		}
	)";

	spoopy_shader_object_t* vert_obj = load_shader(shader_vert, SPOOPY_STAGE_VERTEX);
	spoopy_shader_object_t* frag_obj = load_shader(shader_frag, SPOOPY_STAGE_FRAGMENT);
	spoopy_pipeline_t* pipeline = spoopy_api_pipeline_link(2, (spoopy_shader_object_t*[]){ vert_obj, frag_obj }, 1);

	spoopy_vertex_attr_spec_t vertex_spec[] = {
		{ 2, SPOOPY_VA_FLOAT, SPOOPY_VA_CONV_FLOAT },
		{ 2, SPOOPY_VA_FLOAT, SPOOPY_VA_CONV_FLOAT },
		{ 4, SPOOPY_VA_FLOAT, SPOOPY_VA_CONV_FLOAT }
	};

	spoopy_api_pipeline_compile(pipeline, vert_obj, 3, vertex_spec, 0);
	spoopy_api_shader_destroy(vert_obj);
	spoopy_api_shader_destroy(frag_obj);

	spoopy_mesh_t mesh = {
		.vertex_buffer = NULL,
		.index_buffer = NULL,
		.index_count = 6
	};

	{
		struct vertex2d vertices[] = {
			{ {  1, -1, }, { 1, 0 }, { 1, 1, 1, 1 }, },
			{ {  1,  1, }, { 1, 1 }, { 1, 1, 1, 1 }, },
			{ { -1, -1, }, { 0, 0 }, { 1, 1, 1, 1 }, },
			{ { -1,  1, }, { 0, 1 }, { 1, 1, 1, 1 }, },
		};

		size_t vertex_data_size = sizeof(vertices);
		spoopy_vertex_buffer_t* vbuf = spoopy_api_vertex_buffer_create(vertex_data_size, 4, vertices, 0, pipeline);

		uint16_t indices[] = {
			0, 1, 2,
			2, 1, 3
		};

		spoopy_index_buffer_t* ibuf = spoopy_api_index_buffer_create(6, indices);

		mesh.vertex_buffer = vbuf;
		mesh.index_buffer = ibuf;
	}

	spoopy_texture_t* tex = test_renderer_load_texture("test/tung.png");
	uint32_t u_tex = spoopy_api_pipeline_get_texture_unit(pipeline, "tex0");

	while(!spoopy_api_should_quit()) {
		spoopy_api_begin_frame();
		spoopy_api_clear(SPOOPY_BUFFER_ALL, SPOOPY_RGB(0.0, 0.0, 0.0), 0.0f);
		events_poll(handler_ptr, 0);
		spoopy_api_pipeline_bind(pipeline);
		spoopy_api_texture_set(u_tex, tex);
		spoopy_api_draw_mesh(&mesh, pipeline);
		spoopy_api_swap_buffers();
	}

	return 0;
}
