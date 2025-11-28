#include <spoopy_api.h>
#include <spoopy_backend.h>

#define MAX_MIP_LEVELS 32

spoopy_shader_object_t* spoopy_api_shader_init(spoopy_shader_object_t* shader, spoopy_shader_source_t* info) {
	return _backend_funcs.shader_init(shader, info);
}

void spoopy_api_shader_destroy(spoopy_shader_object_t* shader) {
	_backend_funcs.shader_destroy(shader);
}

spoopy_pipeline_t* spoopy_api_pipeline_link(uint32_t num_objs, spoopy_shader_object_t* objs[], uint32_t num_structs) {
	return _backend_funcs.spoopy_pipeline_link(num_objs, objs, num_structs);
}

uint32_t spoopy_api_pipeline_get_texture_unit(spoopy_pipeline_t* pipeline, const char* name) {
	return _backend_funcs.pipeline_get_texture_unit(pipeline, name);
}

void spoopy_api_pipeline_compile(spoopy_pipeline_t* pipeline, spoopy_shader_object_t* vertex_shader, uint32_t spec_count, spoopy_vertex_attr_spec_t spec[spec_count], uint32_t structure) {
	_backend_funcs.pipeline_compile(pipeline, vertex_shader, spec_count, spec, structure);
}

void spoopy_api_pipeline_bind(spoopy_pipeline_t* pipeline) {
	_backend_funcs.pipeline_bind(pipeline);
}

void spoopy_api_begin_frame(void) {
	_backend_funcs.begin_frame();
}

void spoopy_api_clear(spoopy_buffer_kind_t flags, const spoopy_color_t* color_val, float depth_val) {
	_backend_funcs.clear(flags, color_val, depth_val);
}

void spoopy_api_draw_mesh(const spoopy_mesh_t* mesh, spoopy_pipeline_t* pipeline) {
	_backend_funcs.draw_mesh(mesh, pipeline);
}

void spoopy_api_swap_buffers(void) {
	_backend_funcs.swap_buffers();
}

size_t spoopy_api_texture_size(void) {
	return _backend_funcs.texture_size();
}

void spoopy_api_texture_create(spoopy_texture_t* tex, const spoopy_texture_params_t* p) {
	_backend_funcs.texture_create(tex, p);
}

void spoopy_api_texture_get_size(const spoopy_texture_params_t params, uint32_t mipmap, uint32_t* width, uint32_t* height) {
	if(mipmap >= params.mipmaps) {
		mipmap = params.mipmaps - 1;
	}

	assert(mipmap < MAX_MIP_LEVELS);

	if(mipmap == 0) {
		if(width)  *width  = params.width;
		if(height) *height = params.height;
	}else {
		if(width)  *width  = spoopy_max(1U, params.width >> mipmap);
		if(height) *height = spoopy_max(1U, params.height >> mipmap);
	}
}

void spoopy_api_texture_fill(spoopy_texture_t* tex, uint32_t mipmap, uint32_t layer, const spoopy_image_t* img) {
	_backend_funcs.texture_fill(tex, mipmap, layer, img);
}

void spoopy_api_texture_set(uint32_t unit, spoopy_texture_t* tex) {
	_backend_funcs.texture_set(unit, tex);
}

void spoopy_api_texture_destroy(spoopy_texture_t* tex) {
	_backend_funcs.texture_destroy(tex);
}


// TODO (Optimize Memory) - Make a custom vertex buffer that is universal
// and avoid the need for API wrapper requirement which allows us to privatize
// GPU side such that the RAM usage is minimal and we can recycle buffers
// while also having proper fencing and syncing with the GPU side with a queue system
// to avoid stalls and hitches but reducing RAM usage.
//
// But, also have a proper way to handle dynamic buffers that can grow and shrink
// for other use cases.
spoopy_vertex_buffer_t* spoopy_api_vertex_buffer_create(uint32_t capacity, uint32_t count, void* data, uint32_t structure, spoopy_pipeline_t* pipeline) {
	return _backend_funcs.vertex_buffer_create(capacity, count, data, structure, pipeline);
}

spoopy_index_buffer_t* spoopy_api_index_buffer_create(uint32_t count, void* data) {
	return _backend_funcs.index_buffer_create(count, data);
}
