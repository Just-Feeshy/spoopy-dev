#include <spoopy_api.h>

#include "test_renderer.h"

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

int main(int argc, char** argv) {
    test_init();

	const char *shader_vert = R"(
		#ifndef ShaderTypes_h
		#define ShaderTypes_h
		#endif

		struct VertexInput
		{
			float3 pos : POSITION;
		};

		struct VertexOutput
		{
			float4 position : SV_POSITION;
			float4 color : COLOR0;
		};

		[shader("vertex")]
		VertexOutput vertexMain(VertexInput input)
		{
			VertexOutput output;
			output.position = float4(input.pos.x, input.pos.y, input.pos.z, 1.0);
			output.color = float4(1.0, 0.0, 0.0, 1.0); // Red color
			return output;
		}
	)";

	const char* shader_frag = R"(
		#ifndef ShaderTypes_h
		#define ShaderTypes_h
		#endif

		struct VertexOutput {
			float4 position : SV_Position;
			float4 color : COLOR0;
		};

		[shader("fragment")]
		float4 fragmentMain(VertexOutput input) : SV_Target {
			return input.color;
		}
	)";

	spoopy_shader_object_t* vert_obj = load_shader(shader_vert, SPOOPY_STAGE_VERTEX);
	spoopy_shader_object_t* frag_obj = load_shader(shader_frag, SPOOPY_STAGE_FRAGMENT);
	spoopy_pipeline_t* pipeline = spoopy_api_pipeline_link(2, (spoopy_shader_object_t*[]){ vert_obj, frag_obj }, 1);

	spoopy_vertex_attr_spec_t vertex_spec[] = {
		{ 3, SPOOPY_VA_FLOAT, SPOOPY_VA_CONV_FLOAT }
	};

	spoopy_api_pipeline_compile(pipeline, vert_obj, 1, vertex_spec, 0);
	spoopy_api_shader_destroy(vert_obj);
	spoopy_api_shader_destroy(frag_obj);

	spoopy_mesh_t mesh = {
		.vertex_buffer = NULL,
		.index_buffer = NULL,
		.index_count = 3
	};

	{
		float vertex_data[] = {
			-1.0f, -1.0f, 0.0f,
			 1.0f, -1.0f, 0.0f,
			 0.0f,  1.0f, 0.0f
		};
		size_t vertex_data_size = sizeof(vertex_data);
		spoopy_vertex_buffer_t* vbuf = spoopy_api_vertex_buffer_create(vertex_data_size, 3, vertex_data, 0, pipeline);

		int index_data[] = { 0, 1, 2 };
		spoopy_index_buffer_t* ibuf = spoopy_api_index_buffer_create(3, index_data);

		mesh.vertex_buffer = vbuf;
		mesh.index_buffer = ibuf;
	}

	int frame_count = 0;
	while(!spoopy_api_should_quit()) {
		spoopy_api_begin_frame();
		spoopy_api_clear(SPOOPY_BUFFER_ALL, SPOOPY_RGB(0.0, 0.0, 0.0), 0.0f);
		events_poll(handler_ptr, 0);
		// spoopy_api_draw_mesh(&mesh, pipeline);
		spoopy_api_swap_buffers();
	}

    return 0;
}
