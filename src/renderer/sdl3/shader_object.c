#include <shaderlib/spoopy_defs.h>
#include <SDL3/SDL_gpu.h>

struct spoopy_shader_object {
    SDL_GPUShader* shader;
    spoopy_shader_info_t info;
};
