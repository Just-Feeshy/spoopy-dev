#include <spoopy_api.h>
#include <utils/spoopy_cglm.h>

#include "test_renderer.h"

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

typedef struct vertex_3d {
	float pos[3];
	float uv[2];
	float normal[3];
} vertex_3d_t;

int main(int argc, char** argv) {
	test_init();

	spoopy_fps_counter_t* fps = SPOOPY_INIT_FPS_COUNTER(120, spoopy_stack);

	const char* shader_vert =
		"#ifndef ShaderTypes_h\n"
		"#define ShaderTypes_h\n"
		"#endif\n"
		"\n"
		"cbuffer SpoopyGlobalUniforms\n"
		"{\n"
		"	float4x4 u_model_view;\n"
		"	float4x4 u_mvp;\n"
		"};\n"
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
		"};\n"
		"\n"
		"[shader(\"vertex\")]\n"
		"VertexOutput vertexMain(VertexInput input)\n"
		"{\n"
		"	VertexOutput output;\n"
		"\n"
		"	output.v_position = mul(u_mvp, float4(input.a_pos, 1.0));\n"
		"	output.v_uv = input.a_uv;\n"
		"	output.v_normal = normalize(mul(u_model_view, float4(input.a_normal, 0.0)).xyz);\n"
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
		"};\n"
		"\n"
		"Texture2D tex0;\n"
		"SamplerState samp0;\n"
		"\n"
		"[shader(\"fragment\")]\n"
		"float4 fragmentMain(VertexOutput input) : SV_Target {\n"
		"	float3 norm = normalize(input.v_normal);\n"
		"	float3 light_dir = normalize(float3(0.45, 0.70, 0.55));\n"
		"	float diffuse = max(dot(norm, light_dir), 0.0);\n"
		"	float ambient = 0.20;\n"
		"	float3 albedo = tex0.Sample(samp0, input.v_uv).rgb;\n"
		"	float3 color = albedo * (ambient + 0.80 * diffuse);\n"
		"	return float4(color, 1.0);\n"
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
	spoopy_uniform_t* uniform_model_view = spoopy_api_shader_uniform(pipeline, "u_model_view");
	spoopy_uniform_t* uniform_mvp = spoopy_api_shader_uniform(pipeline, "u_mvp");

	spoopy_vertex_buffer_t* vbuf = spoopy_stack_alloc(spoopy_api_buffer_size(SPOOPY_BUFFER_TYPE_VERTEX));
	spoopy_index_buffer_t* ibuf = spoopy_stack_alloc(spoopy_api_buffer_size(SPOOPY_BUFFER_TYPE_INDEX));
	spoopy_texture_t* tex = NULL;

	spoopy_mesh_t mesh = {
		.vertex_buffers = NULL,
		.index_buffer = NULL,
		.index_count = 3
	};

	{
		vertex_3d_t vertices[] = {
			{ { -1.f,-1.f, 1.f }, { 0.f,1.f }, { 0.f, 0.f, 1.f } },
			{ {  1.f,-1.f, 1.f }, { 1.f,1.f }, { 0.f, 0.f, 1.f } },
			{ {  1.f, 1.f, 1.f }, { 1.f,0.f }, { 0.f, 0.f, 1.f } },
			{ { -1.f, 1.f, 1.f }, { 0.f,0.f }, { 0.f, 0.f, 1.f } },

			{ {  1.f,-1.f,-1.f }, { 0.f,1.f }, { 0.f, 0.f,-1.f } },
			{ { -1.f,-1.f,-1.f }, { 1.f,1.f }, { 0.f, 0.f,-1.f } },
			{ { -1.f, 1.f,-1.f }, { 1.f,0.f }, { 0.f, 0.f,-1.f } },
			{ {  1.f, 1.f,-1.f }, { 0.f,0.f }, { 0.f, 0.f,-1.f } },

			{ { -1.f,-1.f,-1.f }, { 0.f,1.f }, { -1.f,0.f, 0.f } },
			{ { -1.f,-1.f, 1.f }, { 1.f,1.f }, { -1.f,0.f, 0.f } },
			{ { -1.f, 1.f, 1.f }, { 1.f,0.f }, { -1.f,0.f, 0.f } },
			{ { -1.f, 1.f,-1.f }, { 0.f,0.f }, { -1.f,0.f, 0.f } },

			{ {  1.f,-1.f, 1.f }, { 0.f,1.f }, { 1.f, 0.f, 0.f } },
			{ {  1.f,-1.f,-1.f }, { 1.f,1.f }, { 1.f, 0.f, 0.f } },
			{ {  1.f, 1.f,-1.f }, { 1.f,0.f }, { 1.f, 0.f, 0.f } },
			{ {  1.f, 1.f, 1.f }, { 0.f,0.f }, { 1.f, 0.f, 0.f } },

			{ { -1.f, 1.f, 1.f }, { 0.f,1.f }, { 0.f, 1.f, 0.f } },
			{ {  1.f, 1.f, 1.f }, { 1.f,1.f }, { 0.f, 1.f, 0.f } },
			{ {  1.f, 1.f,-1.f }, { 1.f,0.f }, { 0.f, 1.f, 0.f } },
			{ { -1.f, 1.f,-1.f }, { 0.f,0.f }, { 0.f, 1.f, 0.f } },

			{ { -1.f,-1.f,-1.f }, { 0.f,1.f }, { 0.f,-1.f, 0.f } },
			{ {  1.f,-1.f,-1.f }, { 1.f,1.f }, { 0.f,-1.f, 0.f } },
			{ {  1.f,-1.f, 1.f }, { 1.f,0.f }, { 0.f,-1.f, 0.f } },
			{ { -1.f,-1.f, 1.f }, { 0.f,0.f }, { 0.f,-1.f, 0.f } },
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

	tex = test_renderer_load_texture("test/teto.png");
	if(!tex) {
		return 1;
	}

	spoopy_api_texture_set(pipeline, "tex0", tex);

	size_t last_time = spoopy_time_get();
	size_t last_print_time = last_time;
	spoopy_camera_3d_t camera = spoopy_camera_3d_init(VIEWPORT);

	spoopy_api_enable(pipeline, SPOOPY_RCAP_CULL_FACE);
	spoopy_api_cull(pipeline, SPOOPY_CULL_FRONT);
	spoopy_api_blend(pipeline, SPOOPY_BLEND_NONE);

	while(!spoopy_api_should_quit()) {
		size_t t = spoopy_time_get();
		float time_seconds = (float)t / (float)SPOOPY_TIME_RESOLUTION;
		mat4 model_view;
		mat4 rotated;
		mat4 projection;
		mat4 model_view_projection;

		spoopy_events_poll(handler_ptr, 0);
		glm_translate_make(model_view, (vec3){ 0.0f, 0.0f, -5.0f });
		glm_rotate_y(model_view, time_seconds * 0.9f, rotated);
		glm_rotate_x(rotated, -0.55f + 0.25f * sinf(time_seconds * 0.7f), model_view);
		spoopy_camera_get_projection(camera, &projection[0][0]);
		glm_mat4_mul(projection, model_view, model_view_projection);

		spoopy_api_uniform_set_matrix4(uniform_model_view, &model_view[0][0]);
		spoopy_api_uniform_set_matrix4(uniform_mvp, &model_view_projection[0][0]);

		spoopy_api_clear(SPOOPY_BUFFER_COLOR, SPOOPY_RGB(0.0, 0.0, 0.0), 1.0f);
		spoopy_api_pipeline_bind(pipeline);
		spoopy_api_draw_mesh(&mesh, pipeline);
		spoopy_api_swap_buffers();

		spoopy_fps_counter_update(fps);

		if(t - last_print_time > SPOOPY_TIME_RESOLUTION) {
			last_print_time = t;
			SPOOPY_LOG_INFO("%.02f FPS", fps->fps);
		}

		last_time = t;
	}
	spoopy_api_texture_destroy(tex);
	spoopy_heap_free(tex);
	spoopy_api_video_shutdown();
	return 0;
}
