#include <spoopy_api.h>

int main(void) {
	spoopy_vertex_attr_spec_t vertex_spec = {0};
	spoopy_mesh_t mesh = {0};

	spoopy_api_pipeline_compile(NULL, 1, &vertex_spec, 0);
	spoopy_api_pipeline_bind(NULL);
	spoopy_api_draw_mesh(NULL, NULL);
	spoopy_api_draw_mesh(&mesh, NULL);

	spoopy_api_blend(NULL, SPOOPY_BLEND_ALPHA);
	if(spoopy_api_blend_current(NULL) != SPOOPY_BLEND_NONE) {
		return 1;
	}

	spoopy_api_cull(NULL, SPOOPY_CULL_FRONT);
	if(spoopy_api_cull_current(NULL) != SPOOPY_CULL_BACK) {
		return 1;
	}

	spoopy_api_enable(NULL, SPOOPY_RCAP_DEPTH_TEST);
	spoopy_api_disable(NULL, SPOOPY_RCAP_CULL_FACE);
	if(spoopy_api_capabilities_current(NULL) != 0) {
		return 1;
	}

	return 0;
}
