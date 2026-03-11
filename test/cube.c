#include <spoopy_api.h>

#include "test_renderer.h"

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

typedef struct vertex_3d {
	float pos[3];
	float uv[2];
	float normal[3];
} vertex_3d_t;

int main(int argc, char** argv) {
	test_init();

	const char* shader_vert =
		"#ifndef ShaderTypes_h\n"
		"#define ShaderTypes_h\n"
		"#endif\n"
		"\n"
		"struct VertexInput\n"
		"{\n"
		"	float3 a_pos : POSITION;\n"
		"	float2 a_uv : TEXCOORD0;\n"
		"	float3 a_normal : NORMAL;\n"
		"};\n"
		"\n"
		"struct VertexOutput\n"
		"{\n"
		"	float4 v_position : SV_POSITION;\n"
		"	float2 v_uv : TEXCOORD0;\n"
		"	float3 v_normal : TEXCOORD1;\n"
		"	float3 v_fragPos : TEXCOORD2;\n"
		"};\n"
		"\n"
		"uniform float4x4 u_modelViewProjection;\n"
		"uniform float4x4 u_projectionMatrix;\n"
		"\n"
		"[shader(\"vertex\")]\n"
		"VertexOutput vertexMain(VertexInput input)\n"
		"{\n"
		"	VertexOutput output;\n"
		"\n"
		"	float4 posMV = mul(u_modelViewProjection, float4(input.a_pos, 1.0));\n"
		"	output.v_position = mul(u_projectionMatrix, posMV);\n"
		"\n"
		"	output.v_fragPos = posMV.xyz;\n"
		"	output.v_uv = input.a_uv;\n"
		"\n"
		"	float3x3 mv3 = (float3x3)u_modelViewProjection;\n"
		"	float3x3 normalMatrix = transpose(inverse(mv3));\n"
		"	output.v_normal = mul(normalMatrix, input.a_normal);\n"
		"\n"
		"	return output;\n"
		"}\n";

	const char* shader_frag =
		"#ifndef ShaderTypes_h\n"
		"#define ShaderTypes_h\n"
		"#endif\n"
		"\n"
		"struct VertexOutput {\n"
		"	float4 v_position : SV_POSITION;\n"
		"	float2 v_uv : TEXCOORD0;\n"
		"	float3 v_normal : TEXCOORD1;\n"
		"	float3 v_fragPos : TEXCOORD2;\n"
		"};\n"
		"\n"
		"texture2D tex0;\n"
		"sampler samp0;\n"
		"\n"
		"uniform float4 u_lightPos;\n"
		"uniform float4 u_viewPos;\n"
		"\n"
		"[shader(\"fragment\")]\n"
		"float4 fragmentMain(VertexOutput input) : SV_Target {\n"
		"	float3 tex = tex0.Sample(samp0, input.v_uv).xyz;\n"
		"	float3 obj_color = (0.5 + 0.5 * float3(input.v_uv, 0.0)) * tex;\n"
		"\n"
		"	float3 norm = normalize(input.v_normal);\n"
		"	float3 lightDir = normalize(u_lightPos.xyz - input.v_fragPos);\n"
		"\n"
		"	float ndotl = max(dot(norm, lightDir), 0.0);\n"
		"	float3 diffuse = (0.1 + ndotl * u_lightPos.xyz) * obj_color;\n"
		"\n"
		"	return float4(diffuse, 1.0);\n"
		"}\n";

	spoopy_shader_object_t* vert_obj = load_shader(shader_vert, SPOOPY_STAGE_VERTEX);
	spoopy_shader_object_t* frag_obj = load_shader(shader_frag, SPOOPY_STAGE_FRAGMENT);
	spoopy_pipeline_t* pipeline = spoopy_api_pipeline_link(2, (spoopy_shader_object_t*[]){ vert_obj, frag_obj });

	spoopy_vertex_attr_spec_t vertex_spec[] = {
		{ 3, SPOOPY_VA_FLOAT, SPOOPY_VA_CONV_FLOAT },
		{ 2, SPOOPY_VA_FLOAT, SPOOPY_VA_CONV_FLOAT },
		{ 3, SPOOPY_VA_FLOAT, SPOOPY_VA_CONV_FLOAT }
	};

	spoopy_api_pipeline_compile(pipeline, 3, vertex_spec, 0);

	spoopy_vertex_buffer_t* vbuf = spoopy_stack_alloc(spoopy_api_buffer_size(SPOOPY_BUFFER_TYPE_VERTEX));
	spoopy_index_buffer_t* ibuf = spoopy_stack_alloc(spoopy_api_buffer_size(SPOOPY_BUFFER_TYPE_INDEX));

	spoopy_mesh_t mesh = {
		.vertex_buffers = NULL,
		.index_buffer = NULL,
		.index_count = 3
	};

	{
		vertex_3d_t vertices[] = {
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
			0,1,2,    0,2,3,
			4,5,6,    4,6,7,
			8,9,10,   8,10,11,
			12,13,14, 12,14,15,
			16,17,18, 16,18,19,
			20,21,22, 20,22,23
		};

		size_t vertex_data_size = sizeof(vertices);
		spoopy_api_vertex_buffer_create(vbuf, vertex_data_size, ARRAY_SIZE(vertices), vertices, 0);
		spoopy_api_index_buffer_create(ibuf, ARRAY_SIZE(indices), indices);

		mesh.vertex_buffers = vbuf;
		mesh.index_buffer = ibuf;
		mesh.index_count = (uint32_t)ARRAY_SIZE(indices);
		mesh.vertex_count = 1;
	}

	spoopy_uniform_t* u_lightPos = spoopy_api_shader_uniform(pipeline, "u_lightPos");
	spoopy_uniform_t* u_viewPos = spoopy_api_shader_uniform(pipeline, "u_viewPos");

	spoopy_api_uniform_set_float4(u_lightPos, 5.0f, 4.0f, 3.0f, 1.0f);
	spoopy_api_uniform_set_float4(u_viewPos, 0.0f, 0.0f, 5.0f, 1.0f);


	return 0;
}
