#pragma once

#include <spoopy.h>
#include <spoopy_types.h>
#include <spoopy_interp.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum spoopy_camera_3d_projection_type {
	SPOOPY_CAMERA_3D_PROJECTION_PERSPECTIVE,
	SPOOPY_CAMERA_3D_PROJECTION_ORTHOGRAPHIC,
} spoopy_camera_3d_projection_type_t;

typedef struct spoopy_camera_3d {
	float position[3];
	spoopy_interp_float_t _size;
	spoopy_interp_float_t _fov;
	spoopy_interp_float_t _near;
	spoopy_interp_float_t _far;
	spoopy_vec2_int_t viewport;
	spoopy_camera_3d_projection_type_t projection_type;
	spoopy_aspect_axis_t aspect_axis;
} spoopy_camera_3d_t;

SPOOPY_FUNC_CORE spoopy_camera_3d_t spoopy_camera_3d_init(const spoopy_vec2_int_t viewport);
SPOOPY_FUNC_CORE void spoopy_camera_fov_set(spoopy_camera_3d_t* camera, float fov);
SPOOPY_FUNC_CORE void spoopy_camera_near_set(spoopy_camera_3d_t* camera, float fov);
SPOOPY_FUNC_CORE void spoopy_camera_far_set(spoopy_camera_3d_t* camera, float fov);
SPOOPY_FUNC_CORE void spoopy_camera_size_set(spoopy_camera_3d_t* camera, float fov);
SPOOPY_FUNC_CORE void spoopy_camera_get_projection(const spoopy_camera_3d_t camera, float out_matrix[16]);

#ifdef __cplusplus
}
#endif
