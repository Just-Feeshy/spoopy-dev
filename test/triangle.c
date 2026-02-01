#include <spoopy_api.h>

#include "test_renderer.h"

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

int main(int argc, char** argv) {
    test_init();

	// Simple vertex shader - just pass through position
	const char* shader_vert = R"(
		struct VertexInput
		{
			float3 pos : POSITION;
		};

		struct VertexOutput
		{
			float4 position : SV_POSITION;
		};

		[shader("vertex")]
		VertexOutput vertexMain(VertexInput input)
		{
			VertexOutput output;
			output.position = float4(input.pos, 1.0);
			return output;
		}
	)";

	// Simple fragment shader - output solid green
	const char* shader_frag = R"(
		[shader("fragment")]
		float4 fragmentMain() : SV_Target {
			return float4(0.0, 1.0, 0.0, 1.0);
		}
	)";

	spoopy_shader_object_t* vert_obj = load_shader(shader_vert, SPOOPY_STAGE_VERTEX);
	spoopy_shader_object_t* frag_obj = load_shader(shader_frag, SPOOPY_STAGE_FRAGMENT);
	spoopy_pipeline_t* pipeline = spoopy_api_pipeline_link(2, (spoopy_shader_object_t*[]){ vert_obj, frag_obj });

	spoopy_vertex_attr_spec_t vertex_spec[] = {
		{ 3, SPOOPY_VA_FLOAT, SPOOPY_VA_CONV_FLOAT }
	};

	spoopy_api_pipeline_compile(pipeline, 1, vertex_spec, 0);
	spoopy_api_shader_destroy(vert_obj, true);
	spoopy_api_shader_destroy(frag_obj, true);


	spoopy_mesh_t mesh = {
		.vertex_buffers = NULL,
		.index_buffer = NULL,
		.index_count = 3
	};

	{
		float vertex_data[] = {
			-0.75f, -0.75f, 0.0f,
			 0.75f, -0.75f, 0.0f,
			 0.0f,  0.75f, 0.0f
		};

		size_t vertex_data_size = sizeof(vertex_data);
		spoopy_vertex_buffer_t* vbuf = spoopy_stack_alloc(spoopy_api_buffer_size(SPOOPY_BUFFER_TYPE_VERTEX));
		assert(spoopy_api_vertex_buffer_create(vbuf, vertex_data_size, 3, vertex_data, 0));

		uint16_t index_data[] = { 0, 1, 2 };
		spoopy_index_buffer_t* ibuf = spoopy_stack_alloc(spoopy_api_buffer_size(SPOOPY_BUFFER_TYPE_INDEX));
		assert(spoopy_api_index_buffer_create(ibuf, 3, index_data));

		mesh.vertex_buffers = vbuf;
		mesh.index_buffer = ibuf;
		mesh.index_count = 3;
		mesh.vertex_count = 1;
	}

	while(!spoopy_api_should_quit()) {
		spoopy_events_poll(handler_ptr, 0);
		spoopy_api_clear(SPOOPY_BUFFER_COLOR, SPOOPY_RGB(1.0, 0.0, 0.0), 0.0f);
		spoopy_api_pipeline_bind(pipeline);
		spoopy_api_draw_mesh(&mesh, pipeline);
		spoopy_api_swap_buffers();
	}

    return 0;
}
