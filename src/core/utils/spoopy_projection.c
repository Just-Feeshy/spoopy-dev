#include <utils/spoopy_projection.h>
#include <utils/spoopy_misc_math.h>
#include <utils/spoopy_cglm.h>
#include <string.h>

static inline float spoopy_projection_fovy_get(float fov_x, float aspect_ratio) {
	return 2.0f * atanf(tanf(SPOOPY_DEG2RAD_F * fov_x * 0.5f) / aspect_ratio);
}

void spoopy_projection_set_perspective(
	float matrix[16],
	float fov_degrees,
	float aspect_ratio,
	float z_near,
	float z_far,
	bool flip_fov
) {
	float fov_radians = SPOOPY_DEG2RAD_F * fov_degrees;

	if(flip_fov) {
		fov_radians = spoopy_projection_fovy_get(fov_degrees, aspect_ratio);
	}

	mat4 temp;
	glm_perspective(fov_radians, aspect_ratio, z_near, z_far, temp);

	memcpy(matrix, temp, sizeof(float) * 16);
}

void spoopy_projection_set_orthographic(
	float matrix[16],
	float size,
	float aspect_ratio,
	float z_near,
	float z_far,
	bool flip_fov
) {
	if(!flip_fov) {
		size *= aspect_ratio;
	}

	mat4 temp;
	glm_ortho(
		-size * 0.5f,
		size  * 0.5f,
		-size * 0.5f / aspect_ratio,
		size  * 0.5f / aspect_ratio,
		z_near,
		z_far,
		temp
	);

	memcpy(matrix, temp, sizeof(float) * 16);
}
