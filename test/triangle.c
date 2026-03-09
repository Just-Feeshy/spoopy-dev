#define SPOOPY_BUILD_DEBUG
#include <spoopy_api.h>

#include "test_renderer.h"

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

int main(int argc, char** argv) {
    test_init();

	// Simple vertex shader - just pass through position
	const char* shader_vert =
		"#ifndef ShaderTypes_h\n"
		"#define ShaderTypes_h\n"
		"#endif\n"
		"\n"
		"struct VertexInput\n"
		"{\n"
		"    float3 pos : POSITION;\n"
		"};\n"
		"\n"
		"struct VertexOutput\n"
		"{\n"
		"    float4 position : SV_POSITION;\n"
		"    float4 color : COLOR0;\n"
		"};\n"
		"\n"
		"[shader(\"vertex\")]\n"
		"VertexOutput vertexMain(VertexInput input)\n"
		"{\n"
		"    VertexOutput output;\n"
		"    output.position = float4(input.pos.x, input.pos.y, input.pos.z, 1.0);\n"
		"    output.color = float4(ceil(input.pos.x), 1.0 - ceil(input.pos.x), ceil(input.pos.y), 1.0);\n"
		"    return output;\n"
		"}\n";

	const char* shader_frag =
		"#ifndef ShaderTypes_h\n"
		"#define ShaderTypes_h\n"
		"#endif\n"
		"\n"
		"struct VertexOutput {\n"
		"    float4 position : SV_Position;\n"
		"    float4 color : COLOR0;\n"
		"};\n"
		"\n"
		"[shader(\"fragment\")]\n"
		"float4 fragmentMain(VertexOutput input) : SV_Target {\n"
		"    return input.color;\n"
		"}\n";

	spoopy_shader_object_t* vert_obj = load_shader(shader_vert, SPOOPY_STAGE_VERTEX);
	spoopy_shader_object_t* frag_obj = load_shader(shader_frag, SPOOPY_STAGE_FRAGMENT);
	spoopy_pipeline_t* pipeline = spoopy_api_pipeline_link(2, (spoopy_shader_object_t*[]){ vert_obj, frag_obj });

	spoopy_vertex_attr_spec_t vertex_spec[] = {
		{ 3, SPOOPY_VA_FLOAT, SPOOPY_VA_CONV_FLOAT }
	};

	spoopy_api_pipeline_compile(pipeline, 1, vertex_spec, 0);
	spoopy_api_shader_destroy(vert_obj, true);
	spoopy_api_shader_destroy(frag_obj, true);


	float vertex_data[] = {
		-0.75f, -0.75f, 0.0f,
		 0.75f, -0.75f, 0.0f,
		 0.0f,  0.75f, 0.0f
	};

	uint16_t index_data[] = { 0, 1, 2 };

	spoopy_vertex_buffer_t* vbuf = spoopy_stack_alloc(spoopy_api_buffer_size(SPOOPY_BUFFER_TYPE_VERTEX));
	spoopy_index_buffer_t* ibuf = spoopy_stack_alloc(spoopy_api_buffer_size(SPOOPY_BUFFER_TYPE_INDEX));

	if(!spoopy_api_vertex_buffer_create(vbuf, sizeof(vertex_data), 3, vertex_data, 0)) {
		SPOOPY_LOG_ERROR("Failed to create vertex buffer");
		return 1;
	}

	if(!spoopy_api_index_buffer_create(ibuf, 3, index_data)) {
		SPOOPY_LOG_ERROR("Failed to create index buffer");
		return 1;
	}

	spoopy_mesh_t mesh = {
		.vertex_buffers = vbuf,
		.index_buffer = ibuf,
		.index_count = 3,
		.vertex_count = 1
	};

	while(!spoopy_api_should_quit()) {
		spoopy_events_poll(handler_ptr, 0);
		spoopy_api_clear(SPOOPY_BUFFER_COLOR, SPOOPY_RGB(0.0, 0.0, 0.0), 0.0f);
		spoopy_api_pipeline_bind(pipeline);
		spoopy_api_draw_mesh(&mesh, pipeline);
		spoopy_api_swap_buffers();
	}

	spoopy_api_video_shutdown();
    return 0;
}
