#include <spoopy_api.h>

#include "test_renderer.h"

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

int main(int argc, char** argv) {
	test_init();

	const char* shader_vert = R"(
		#ifndef ShaderTypes_h
		#define ShaderTypes_h
		#endif

		struct VertexInput
		{
			float3 a_pos : POSITION;
			float2 a_uv : TEXCOORD0;
			float3 a_normal : NORMAL;
		};

		struct VertexOutput
		{
			float4 v_position : SV_POSITION;
			float2 v_uv : TEXCOORD0;
			float3 v_normal : TEXCOORD1;
			float3 v_fragPos : TEXCOORD2;
		};

		cbuffer Globals
		{
			float4x4 u_modelViewProjection;
			float4x4 u_projectionMatrix;
		};

		[shader("vertex")]
		VertexOutput vertexMain(VertexInput input)
		{
			VertexOutput output;

			float4 posMV = mul(u_modelViewProjection, float4(input.a_pos, 1.0));
			output.v_position = mul(u_projectionMatrix, posMV);

			output.v_fragPos = posMV.xyz;
			output.v_uv = input.a_uv;

			float3x3 mv3 = (float3x3)u_modelViewProjection;
			float3x3 normalMatrix = transpose(inverse(mv3));
			output.v_normal = mul(normalMatrix, input.a_normal);

			return output;
		}
	)";

	const char* shader_frag = R"(
		#ifndef ShaderTypes_h
		#define ShaderTypes_h
		#endif

		struct VertexOutput {
			float4 v_position : SV_POSITION;
			float2 v_uv : TEXCOORD0;
			float3 v_normal : TEXCOORD1;
			float3 v_fragPos : TEXCOORD2;
		};

		texture2D tex0;
		sampler samp0;

		cbuffer Light {
			float4 u_lightPos;
			float4 u_viewPos;
		};

		[shader("fragment")]
		float4 fragmentMain(VertexOutput input) : SV_Target {
			float3 tex = tex0.Sample(samp0, input.v_uv).xyz;
			float3 obj_color = (0.5 + 0.5 * float3(input.v_uv, 0.0)) * tex;

			float3 norm = normalize(input.v_normal);
			float3 lightDir = normalize(u_lightPos.xyz - input.v_fragPos);

			float ndotl = max(dot(norm, lightDir), 0.0);
			float3 diffuse = (0.1 + ndotl * u_lightPos.xyz) * obj_color;

			return float4(diffuse, 1.0);
		}
)";

	spoopy_shader_object_t* vert_obj = load_shader(shader_vert, SPOOPY_STAGE_VERTEX);
	spoopy_shader_object_t* frag_obj = load_shader(shader_frag, SPOOPY_STAGE_FRAGMENT);
	spoopy_pipeline_t* pipeline = spoopy_api_pipeline_link(2, (spoopy_shader_object_t*[]){ vert_obj, frag_obj }, 1);

	spoopy_vertex_attr_spec_t vertex_spec[] = {
		{ 3, SPOOPY_VA_FLOAT, SPOOPY_VA_CONV_FLOAT },
		{ 2, SPOOPY_VA_FLOAT, SPOOPY_VA_CONV_FLOAT },
		{ 3, SPOOPY_VA_FLOAT, SPOOPY_VA_CONV_FLOAT }
	};

	spoopy_api_pipeline_compile(pipeline, vert_obj, 3, vertex_spec, 0);
	spoopy_api_shader_destroy(vert_obj);
	spoopy_api_shader_destroy(frag_obj);

	spoopy_mesh_t mesh = {
		.vertex_buffer = NULL,
		.index_buffer = NULL,
		.index_count = 3
	};

	{
		vertex3d_t vertices[] = {
			{ { -1.f,-1.f, 1.f }, { 0.f,0.f }, { 0.f, 0.f, 1.f } },
			{ {  1.f,-1.f, 1.f }, { 1.f,0.f }, { 0.f, 0.f, 1.f } },
			{ {  1.f, 1.f, 1.f }, { 1.f,1.f }, { 0.f, 0.f, 1.f } },
			{ { -1.f, 1.f, 1.f }, { 0.f,1.f }, { 0.f, 0.f, 1.f } },

			{ {  1.f,-1.f,-1.f }, { 0.f,0.f }, { 0.f, 0.f,-1.f } },
			{ { -1.f,-1.f,-1.f }, { 1.f,0.f }, { 0.f, 0.f,-1.f } },
			{ { -1.f, 1.f,-1.f }, { 1.f,1.f }, { 0.f, 0.f,-1.f } },
			{ {  1.f, 1.f,-1.f }, { 0.f,1.f }, { 0.f, 0.f,-1.f } },

			{ { -1.f,-1.f,-1.f }, { 0.f,0.f }, { -1.f,0.f, 0.f } },
			{ { -1.f,-1.f, 1.f }, { 1.f,0.f }, { -1.f,0.f, 0.f } },
			{ { -1.f, 1.f, 1.f }, { 1.f,1.f }, { -1.f,0.f, 0.f } },
			{ { -1.f, 1.f,-1.f }, { 0.f,1.f }, { -1.f,0.f, 0.f } },

			{ {  1.f,-1.f, 1.f }, { 0.f,0.f }, { 1.f, 0.f, 0.f } },
			{ {  1.f,-1.f,-1.f }, { 1.f,0.f }, { 1.f, 0.f, 0.f } },
			{ {  1.f, 1.f,-1.f }, { 1.f,1.f }, { 1.f, 0.f, 0.f } },
			{ {  1.f, 1.f, 1.f }, { 0.f,1.f }, { 1.f, 0.f, 0.f } },

			{ { -1.f, 1.f, 1.f }, { 0.f,0.f }, { 0.f, 1.f, 0.f } },
			{ {  1.f, 1.f, 1.f }, { 1.f,0.f }, { 0.f, 1.f, 0.f } },
			{ {  1.f, 1.f,-1.f }, { 1.f,1.f }, { 0.f, 1.f, 0.f } },
			{ { -1.f, 1.f,-1.f }, { 0.f,1.f }, { 0.f, 1.f, 0.f } },

			{ { -1.f,-1.f,-1.f }, { 0.f,0.f }, { 0.f,-1.f, 0.f } },
			{ {  1.f,-1.f,-1.f }, { 1.f,0.f }, { 0.f,-1.f, 0.f } },
			{ {  1.f,-1.f, 1.f }, { 1.f,1.f }, { 0.f,-1.f, 0.f } },
			{ { -1.f,-1.f, 1.f }, { 0.f,1.f }, { 0.f,-1.f, 0.f } },
		};

		uint16_t indices[] = {
			0,1,2, 0,2,3,
			4,5,6, 4,6,7,
			8,9,10, 8,10,11,
			12,13,14, 12,14,15,
			16,17,18, 16,18,19,
			20,21,22, 20,22,23
		};

		size_t vertex_data_size = sizeof(vertices);
		spoopy_vertex_buffer_t* vbuf = spoopy_api_vertex_buffer_create(vertex_data_size, ARRAY_SIZE(vertices), vertices, 0, pipeline);
		spoopy_index_buffer_t* ibuf = spoopy_api_index_buffer_create(ARRAY_SIZE(indices), indices);

		mesh.vertex_buffer = vbuf;
		mesh.index_buffer = ibuf;
		mesh.index_count = (uint32_t)ARRAY_SIZE(indices);
	}

	uint32_t u_tex = spoopy_api_pipeline_get_texture_unit(pipeline, "tex0");
	spoopy_uniform_t u_lightPos = spoopy_api_shader_uniform(pipeline, "u_lightPos");
	spoopy_uniform_t u_viewPos = spoopy_api_shader_uniform(pipeline, "u_viewPos");

	return 0;
}
