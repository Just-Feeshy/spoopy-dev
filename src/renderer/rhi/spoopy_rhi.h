#ifndef SPOOPY_RHI_H
#define SPOOPY_RHI_H

#include "../spoopy_build_config.h"

typedef struct spoopy_renderer_funcs {
    void (*init)(void);
} spoopy_renderer_funcs_t;

typedef struct spoopy_renderer_bootstrap {
    const char* name;
    spoopy_renderer_funcs_t funcs;
} spoopy_renderer_bootstrap_t;

#ifdef __cplusplus
extern "C" {
#endif

#undef R
#define R(x) extern spoopy_renderer_bootstrap_t spoopy_##x##_backend;
SPOOPY_HAS_RENDERER_BACKENDS
#undef R

void spoopy_new_rhi(void);

#ifdef __cplusplus
}
#endif // extern "C"

#endif // SPOOPY_RHI_H
