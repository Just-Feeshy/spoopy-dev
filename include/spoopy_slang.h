#pragma once

// A more controlled way to handle Slang compilation targets
// without going the extra mile and having to have switch statements
// or if-else chains everywhere in the codebase.

#ifndef SPOOPY_NO_HEADER_SLANG

typedef enum {
    SLANG_SPIRV,
    SLANG_SPIRV_ASM,
    SLANG_HLSL,
    SLANG_DXBC,
    SLANG_DXBC_ASM,
    SLANG_DXIL,
    SLANG_DXIL_ASM,
    SLANG_METAL_LIB,
    SLANG_METAL_LIB_ASM,
} SlangCompileTarget;

#endif // SPOOPY_NO_HEADER_SLANG

#ifdef __cplusplus
extern "C" {
#endif

extern uint32_t spoopy_slang_family;

#ifdef __cplusplus
}
#endif // extern "C"
