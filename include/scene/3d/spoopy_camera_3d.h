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
	spoopy_interp_float_t _size;
	spoopy_interp_float_t _fov;
	spoopy_interp_float_t _near;
	spoopy_interp_float_t _far;
	float position[3];
	spoopy_camera_3d_projection_type_t projection_type;
	spoopy_aspect_axis_t aspect_axis;
} spoopy_camera_3d_t;

SPOOPY_FUNC_CORE spoopy_camera_3d_t spoopy_camera_3d_init(void);
SPOOPY_FUNC_CORE void spoopy_camera_fov_set(spoopy_camera_3d_t* camera, float fov);

#ifdef __cplusplus
}
#endif
