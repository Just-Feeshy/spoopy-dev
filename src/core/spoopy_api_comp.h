#pragma once

#include <spoopy.h>
#include <spoopy_log.h>
#include <spoopy_types.h>

typedef enum spoopy_api_warn_once_flag {
	SPOOPY_API_WARN_NULL_PIPELINE_COMPILE = 1u << 0,
	SPOOPY_API_WARN_NULL_PIPELINE_BIND = 1u << 1,
	SPOOPY_API_WARN_NULL_PIPELINE_DRAW = 1u << 2,
	SPOOPY_API_WARN_NULL_PIPELINE_BLEND_UPDATE = 1u << 3,
	SPOOPY_API_WARN_NULL_PIPELINE_BLEND_QUERY = 1u << 4,
	SPOOPY_API_WARN_NULL_PIPELINE_CULL_UPDATE = 1u << 5,
	SPOOPY_API_WARN_NULL_PIPELINE_CULL_QUERY = 1u << 6,
	SPOOPY_API_WARN_NULL_PIPELINE_CAPABILITY_QUERY = 1u << 7,
	SPOOPY_API_WARN_NULL_PIPELINE_CAPABILITY_UPDATE = 1u << 8,
	SPOOPY_API_WARN_NULL_MESH_DRAW = 1u << 9,
} spoopy_api_warn_once_flag_t;

bool spoopy_api_require_pipeline(
	const spoopy_pipeline_t* pipeline,
	spoopy_api_warn_once_flag_t warn_flag,
	const char* action
);

bool spoopy_api_require_mesh(
	const spoopy_mesh_t* mesh,
	spoopy_api_warn_once_flag_t warn_flag,
	const char* action
);
