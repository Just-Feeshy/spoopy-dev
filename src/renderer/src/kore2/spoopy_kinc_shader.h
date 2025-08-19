#ifndef SPOOPY_KORE_SHADER_H
#define SPOOPY_KORE_SHADER_H

#include <spoopy_shader.h>
#include <kinc/graphics5/shader.h>

struct spoopy_shader_object {
	kinc_g5_shader_t core;
};

void spoopy_kinc_shader_init(spoopy_shader_object_t* shader, spoopy_shader_source_t* info);

#endif
