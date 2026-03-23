#include <spoopy_api.h>
#include <spoopy_backend.h>
#include "spoopy_api_comp.h"


// My dumbass always forgets to create a uniform function
// and waste time figuring out why a segfault occured
// Also `if` parsing is annoying
#define SPOOPY_CALL_UNIFORM_FN(fn, ...) \
    do { \
        if(_backend_funcs.fn == NULL) { \
            SPOOPY_LOG_ERROR("Uniform function '%s' not available on this backend.", #fn); \
            return; \
        } \
        _backend_funcs.fn(__VA_ARGS__); \
    } while(0)

#define MAX_MIP_LEVELS 32

bool spoopy_api_shader_init(spoopy_shader_object_t* shader, spoopy_shader_source_t* info) {
	return _backend_funcs.shader_init(shader, info);
}

void spoopy_api_shader_destroy(spoopy_shader_object_t* shader, bool must_free) {
	_backend_funcs.shader_destroy(shader, must_free);

	if(must_free) {
		spoopy_heap_free(shader);
	}
}

spoopy_pipeline_t* spoopy_api_pipeline_link(uint32_t num_objs, spoopy_shader_object_t* objs[]) {
	return _backend_funcs.pipeline_link(num_objs, objs);
}

void spoopy_api_pipeline_compile(spoopy_pipeline_t* pipeline, uint32_t spec_count, spoopy_vertex_attr_spec_t spec[spec_count], uint32_t buffer_index) {
	if(!spoopy_api_require_pipeline(pipeline, SPOOPY_API_WARN_NULL_PIPELINE_COMPILE, "Pipeline compile request")) {
		return;
	}

	_backend_funcs.pipeline_compile(pipeline, spec_count, spec, buffer_index);
}

void spoopy_api_pipeline_bind(spoopy_pipeline_t* pipeline) {
	if(!spoopy_api_require_pipeline(pipeline, SPOOPY_API_WARN_NULL_PIPELINE_BIND, "Pipeline bind request")) {
		return;
	}

	_backend_funcs.pipeline_bind(pipeline);
}

void spoopy_api_draw_mesh(const spoopy_mesh_t* mesh, spoopy_pipeline_t* pipeline) {
	if(!spoopy_api_require_mesh(mesh, SPOOPY_API_WARN_NULL_MESH_DRAW, "Draw request")) {
		return;
	}

	if(!spoopy_api_require_pipeline(pipeline, SPOOPY_API_WARN_NULL_PIPELINE_DRAW, "Draw request")) {
		return;
	}

	_backend_funcs.draw_mesh(mesh, pipeline);
}

void spoopy_api_swap_buffers(void) {
	_backend_funcs.swap_buffers();
}

void spoopy_api_blend(spoopy_pipeline_t* pipeline, spoopy_blend_mode_t mode) {
	if(!spoopy_api_require_pipeline(pipeline, SPOOPY_API_WARN_NULL_PIPELINE_BLEND_UPDATE, "Blend state update")) {
		return;
	}

	_backend_funcs.blend(pipeline, mode);
}

spoopy_blend_mode_t spoopy_api_blend_current(spoopy_pipeline_t* pipeline) {
	if(!spoopy_api_require_pipeline(pipeline, SPOOPY_API_WARN_NULL_PIPELINE_BLEND_QUERY, "Blend state query")) {
		return SPOOPY_BLEND_NONE;
	}

	return _backend_funcs.blend_current(pipeline);
}

void spoopy_api_cull(spoopy_pipeline_t* pipeline, spoopy_cull_face_mode_t mode) {
	if(!spoopy_api_require_pipeline(pipeline, SPOOPY_API_WARN_NULL_PIPELINE_CULL_UPDATE, "Cull state update")) {
		return;
	}

	_backend_funcs.cull(pipeline, mode);
}

spoopy_cull_face_mode_t spoopy_api_cull_current(spoopy_pipeline_t* pipeline) {
	if(!spoopy_api_require_pipeline(pipeline, SPOOPY_API_WARN_NULL_PIPELINE_CULL_QUERY, "Cull state query")) {
		return SPOOPY_CULL_BACK;
	}

	return _backend_funcs.cull_current(pipeline);
}

void spoopy_api_texture_create(spoopy_texture_t* tex, const spoopy_texture_params_t* p) {
	_backend_funcs.texture_create(tex, p);
}

size_t spoopy_api_texture_size(void) {
	return _backend_funcs.texture_size();
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

void spoopy_api_texture_set(spoopy_pipeline_t* pipeline, const char* uniform_name, spoopy_texture_t* tex) {
	_backend_funcs.texture_set(pipeline, uniform_name, tex);
}

void spoopy_api_texture_destroy(spoopy_texture_t* tex) {
	_backend_funcs.texture_destroy(tex);
}

size_t spoopy_api_buffer_size(spoopy_buffer_type_t type) {
	return _backend_funcs.buffer_size(type);
}

spoopy_uniform_t* spoopy_api_shader_uniform(spoopy_pipeline_t* pipeline, const char* name) {
	return _backend_funcs.shader_uniform(pipeline, name);
}

void spoopy_api_uniform_set_int(spoopy_uniform_t* uniform, int value) {
	SPOOPY_CALL_UNIFORM_FN(uniform_set_int, uniform, value);
}

void spoopy_api_uniform_set_int2(spoopy_uniform_t* uniform, int value0, int value1) {
	SPOOPY_CALL_UNIFORM_FN(uniform_set_int2, uniform, value0, value1);
}

void spoopy_api_uniform_set_int3(spoopy_uniform_t* uniform, int value0, int value1, int value2) {
	SPOOPY_CALL_UNIFORM_FN(uniform_set_int3, uniform, value0, value1, value2);
}

void spoopy_api_uniform_set_int4(spoopy_uniform_t* uniform, int value0, int value1, int value2, int value3) {
	SPOOPY_CALL_UNIFORM_FN(uniform_set_int4, uniform, value0, value1, value2, value3);
}

void spoopy_api_uniform_set_ints(spoopy_uniform_t* uniform, const int* values, int count) {
	SPOOPY_CALL_UNIFORM_FN(uniform_set_ints, uniform, values, count);
}

void spoopy_api_uniform_set_float(spoopy_uniform_t* uniform, float value) {
	SPOOPY_CALL_UNIFORM_FN(uniform_set_float, uniform, value);
}

void spoopy_api_uniform_set_float2(spoopy_uniform_t* uniform, float value0, float value1) {
	SPOOPY_CALL_UNIFORM_FN(uniform_set_float2, uniform, value0, value1);
}

void spoopy_api_uniform_set_float3(spoopy_uniform_t* uniform, float value0, float value1, float value2) {
	SPOOPY_CALL_UNIFORM_FN(uniform_set_float3, uniform, value0, value1, value2);
}

void spoopy_api_uniform_set_float4(spoopy_uniform_t* uniform, float value0, float value1, float value2, float value3) {
	SPOOPY_CALL_UNIFORM_FN(uniform_set_float4, uniform, value0, value1, value2, value3);
}

void spoopy_api_uniform_set_floats(spoopy_uniform_t* uniform, const float* values, int count) {
	SPOOPY_CALL_UNIFORM_FN(uniform_set_floats, uniform, values, count);
}

void spoopy_api_uniform_set_bool(spoopy_uniform_t* uniform, bool value) {
	SPOOPY_CALL_UNIFORM_FN(uniform_set_bool, uniform, value);
}

void spoopy_api_uniform_set_matrix3(spoopy_uniform_t* uniform, const float* values) {
	SPOOPY_CALL_UNIFORM_FN(uniform_set_matrix3, uniform, values);
}

void spoopy_api_uniform_set_matrix4(spoopy_uniform_t* uniform, const float* values) {
	SPOOPY_CALL_UNIFORM_FN(uniform_set_matrix4, uniform, values);
}

spoopy_capability_bits_t spoopy_api_capabilities_current(spoopy_pipeline_t* pipeline) {
	if(!spoopy_api_require_pipeline(pipeline, SPOOPY_API_WARN_NULL_PIPELINE_CAPABILITY_QUERY, "Render capability query")) {
		return 0;
	}

	return _backend_funcs.capabilities_current(pipeline);
}

spoopy_capability_bits_t spoopy_capability_bit(spoopy_render_capability_t cap) {
	spoopy_capability_bits_t idx = cap;
	assert(idx < SPOOPY_NUM_RCAPS);
	return (1 << idx);
}

void spoopy_api_capability(spoopy_pipeline_t* pipeline, spoopy_render_capability_t cap, bool value) {
	if(!spoopy_api_require_pipeline(pipeline, SPOOPY_API_WARN_NULL_PIPELINE_CAPABILITY_UPDATE, "Render capability update")) {
		return;
	}

	spoopy_capability_bits_t caps = spoopy_api_capabilities_current(pipeline), new_caps;

	if(value) {
		new_caps = caps | spoopy_capability_bit(cap);
	}else {
		new_caps = caps & ~spoopy_capability_bit(cap);
	}

	if(caps != new_caps) {
		// TODO (Events): Have somekind of state notification thingy, or something..
		_backend_funcs.capabilities(pipeline, new_caps);
	}
}

void spoopy_api_enable(spoopy_pipeline_t* pipeline, spoopy_render_capability_t cap) {
	spoopy_api_capability(pipeline, cap, true);
}

void spoopy_api_disable(spoopy_pipeline_t* pipeline, spoopy_render_capability_t cap) {
	spoopy_api_capability(pipeline, cap, false);
}


// TODO (Optimize Memory) - Make a custom vertex buffer that is universal
// and avoid the need for API wrapper requirement which allows us to privatize
// GPU side such that the RAM usage is minimal and we can recycle buffers
// while also having proper fencing and syncing with the GPU side with a queue system
// to avoid stalls and hitches but reducing RAM usage.
//
// But, also have a proper way to handle dynamic buffers that can grow and shrink
// for other use cases.
bool spoopy_api_vertex_buffer_create(spoopy_vertex_buffer_t* buffer, uint32_t capacity, uint32_t count, void* data, uint32_t stride) {
	return _backend_funcs.vertex_buffer_create(buffer, capacity, count, data, stride);
}

bool spoopy_api_index_buffer_create(spoopy_index_buffer_t* buffer, uint32_t count, void* data) {
	return _backend_funcs.index_buffer_create(buffer, count, data);
}
