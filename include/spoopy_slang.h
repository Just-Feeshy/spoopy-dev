#ifndef SPOOPY_SLANG_H
#define SPOOPY_SLANG_H

// A more controlled way to handle Slang compilation targets
// without going the extra mile and having to have switch statements
// or if-else chains everywhere in the codebase.

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

#endif
