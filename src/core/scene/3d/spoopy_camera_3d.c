#include <spoopy_camera_3d.h>
#include <spoopy_projection.h>
#include <spoopy_log.h>

spoopy_camera_3d_t spoopy_camera_3d_init(void) {
	spoopy_camera_3d_t camera;

	camera.projection_type = SPOOPY_CAMERA_3D_PROJECTION_PERSPECTIVE;
	camera.aspect_axis = SPOOPY_ASPECT_AXIS_HEIGHT;

	camera.position[0] = 0.0f;
	camera.position[1] = 0.0f;
	camera.position[2] = 0.0f;

	camera._fov = spoopy_interp_float_init();
	camera._size = spoopy_interp_float_init();
	camera._near = spoopy_interp_float_init();
	camera._far = spoopy_interp_float_init();

	spoopy_interp_float_set(&camera._fov, 60.0f);
	spoopy_interp_float_set(&camera._near, 0.05f);
	spoopy_interp_float_set(&camera._far, 1000.0f);
	spoopy_interp_float_set(&camera._size, 1.0f);

	return camera;
}

void spoopy_camera_fov_set(spoopy_camera_3d_t* camera, float fov) {
	SPOOPY_LOG_ERR_COND(fov < 1.0f || fov > 179.0f);
	spoopy_interp_float_set(&camera->_fov, fov);
}

void spoopy_camera_near_set(spoopy_camera_3d_t* camera, float z_near) {
	spoopy_interp_float_set(&camera->_near, z_near);
}

void spoopy_camera_far_set(spoopy_camera_3d_t* camera, float z_far) {
	spoopy_interp_float_set(&camera->_far, z_far);
}

void spoopy_camera_size_set(spoopy_camera_3d_t* camera, float size) {
	SPOOPY_LOG_ERR_COND(size <= SPOOPY_EPSILON);
	spoopy_interp_float_set(&camera->_size, size);
}
