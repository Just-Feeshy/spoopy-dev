#pragma once

#include <spoopy.h>

void spoopy_projection_set_perspective(float matrix[16], float fov_degrees, float aspect_ratio, float z_near, float z_far, bool flip_fov);
void spoopy_projection_set_orthographic(float matrix[16], float size, float aspect_ratio, float z_near, float z_far, bool flip_fov);
