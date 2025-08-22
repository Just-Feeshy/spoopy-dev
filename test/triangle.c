#include <spoopy_api.h>
#include <kinc/graphics4/pipeline.h>
#include <kinc/graphics4/vertexbuffer.h>
#include <kinc/graphics4/graphics.h>
#include <kinc/graphics4/indexbuffer.h>

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
			float3 position : POSITION;
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
			output.position = float4(input.position.x, input.position.y, input.position.z, 1.0);
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

	kinc_g4_vertex_buffer_t vertices;
	kinc_g4_index_buffer_t indices;

	spoopy_shader_object_t* vert_obj = load_shader(shader_vert, SPOOPY_STAGE_VERTEX);
	spoopy_shader_object_t* frag_obj = load_shader(shader_frag, SPOOPY_STAGE_FRAGMENT);
	spoopy_pipeline_t* pipeline = spoopy_api_pipeline_link(2, (spoopy_shader_object_t*[]){ vert_obj, frag_obj }, 1);

	spoopy_vertex_attr_spec_t vertex_spec[] = {
		{ "position_0", 3, SPOOPY_VA_FLOAT, SPOOPY_VA_CONV_FLOAT }
	};

	spoopy_api_pipeline_compile(pipeline, 1, vertex_spec, 0);

	// while(!spoopy_api_should_quit()) {
		// kinc_g4_begin(0);
		// kinc_g4_clear(KINC_G4_CLEAR_COLOR, 0, 0.0f, 0);

		// kinc_g4_set_pipeline(&pipeline);
		// kinc_g4_set_vertex_buffer(&vertices);
		// kinc_g4_set_index_buffer(&indices);
		// kinc_g4_draw_indexed_vertices();

		// kinc_g4_end(0);
		// kinc_g4_swap_buffers();
	// }

    return 0;
}
