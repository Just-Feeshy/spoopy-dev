#ifndef SPOOPY_SPOOPY_SHADER_H
#define SPOOPY_SPOOPY_SHADER_H

#include <spoopy.h>
#include <shaderlib/defs.h>

#ifdef __cplusplus
extern "C" {
#endif

bool spoopy_shader_language_supported(
        const spoopy_shader_info_t* info
);

spoopy_shader_language_t spoopy_shader_required_language();

#ifdef __cplusplus
}
#endif

#endif // SPOOPY_SPOOPY_SHADER_H
