#pragma once

#include <spoopy_log.h>

#import <QuartzCore/CAMetalLayer.h>
#import <Metal/Metal.h>

#ifdef __cplusplus
extern "C" {
#endif

// Returns the shared Metal device (creates it on first call)
id<MTLDevice> spoopy_metal_init(void);

#ifdef __cplusplus
}
#endif
