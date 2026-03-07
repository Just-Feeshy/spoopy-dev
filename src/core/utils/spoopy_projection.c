#include <utils/spoopy_projection.h>
#include <spoopy_misc_math.h>
#include <spoopy_cglm.h>

static inline float spoopy_projection_fovy_get(float fov_x, float aspect_ratio) {
	return atanf(aspect_ratio * atanf(SPOOPY_DEG2RAD_F * fov_x * 0.5)) * 2.0f;
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

	glm_perspective(fov_radians, aspect_ratio, z_near, z_far, matrix);
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

	glm_ortho(
		-size * 0.5f,
		size  * 0.5f,
		-size * 0.5f / aspect_ratio,
		size  * 0.5f / aspect_ratio,
		z_near,
		z_far,
		matrix
	);
}
