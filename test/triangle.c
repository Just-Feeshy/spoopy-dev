#include <spoopy_api.h>

#include "test_renderer.h"

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

int main(int argc, char** argv) {
    test_init();

	const char *shader_vert = R"(
		struct VertexInput
		{
			float2 position : POSITION;
			float4 color : COLOR0;
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
			output.position = float4(input.position.x, input.position.y, 0.0, 1.0);
			output.color = input.color;
			return output;
		}
	)";

	const char* shader_frag = R"(
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

    return 0;
}
