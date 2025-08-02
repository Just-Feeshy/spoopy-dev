#include <spoopy.h>
#include <spoopy_video.h>
#include <kore3/gpu/device.h>
#include <kore3/system.h>
#include <kong.h>

// Needs to be in a RHI
static kore_gpu_device device;
static kore_gpu_command_list command_list;

// Needs to be managed better
static vertex_in_buffer vertices;
static kore_gpu_buffer indices;


void spoopy_video_init(const spoopy_video_init_params_t* params) {
    kore_init(params->title, params->width, params->height, NULL, NULL);

    kore_gpu_device_wishlist wishlist = {0};
    kore_gpu_device_create(&device, &wishlist);

    kong_init(&device);

    kore_gpu_device_create_command_list(&device, KORE_GPU_COMMAND_LIST_TYPE_GRAPHICS, &command_list);

    kong_create_buffer_vertex_in(&device, 3, &vertices);
    vertex_in *v = kong_vertex_in_buffer_lock(&vertices);

    v[0].pos.x = -1.0f;
	v[0].pos.y = -1.0f;
	v[0].pos.z = 0.5f;

	v[1].pos.x = 1.0f;
	v[1].pos.y = -1.0f;
	v[1].pos.z = 0.5f;

	v[2].pos.x = -1.0f;
	v[2].pos.y = 1.0f;
	v[2].pos.z = 0.5f;

    kong_vertex_in_buffer_unlock(&vertices);

    kore_gpu_buffer_parameters params = {
	    .size        = 3 * sizeof(uint16_t),
	    .usage_flags = KORE_GPU_BUFFER_USAGE_INDEX | KORE_GPU_BUFFER_USAGE_CPU_WRITE,
	};
	kore_gpu_device_create_buffer(&device, &params, &indices);
	{
		uint16_t *i = (uint16_t *)kore_gpu_buffer_lock_all(&indices);

		i[0] = 0;
		i[1] = 1;
		i[2] = 2;

		kore_gpu_buffer_unlock(&indices);
	}
}

void spoopy_video_shutdown(void) {
    // Cleanup code here
}
