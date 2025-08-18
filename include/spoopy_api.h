#ifndef SPOOPY_API_H
#define SPOOPY_API_H

// For more general API functions for the user to use.
// Everything here is meant to be used by the developer

#define SPOOPY_MAIN_PROJECT
#include <spoopy.h>

#ifdef __cplusplus
extern "C" {
#endif

bool spoopy_api_shader_supported(spoopy_context_t* context);

#ifdef __cplusplus
}
#endif // extern "C"

#undef SPOOPY_MAIN_PROJECT

#endif
