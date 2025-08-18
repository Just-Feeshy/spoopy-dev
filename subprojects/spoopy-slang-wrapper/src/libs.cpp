#include <slang-com-ptr.h>
#include <slang.h>
#include <spoopy_log.h>
#include <spoopy_shader.h>

static_assert(SPOOPY_OPTIMIZATION_LEVEL_NONE == (int)SLANG_OPTIMIZATION_LEVEL_NONE, "");
static_assert(SPOOPY_OPTIMIZATION_LEVEL_DEFAULT == (int)SLANG_OPTIMIZATION_LEVEL_DEFAULT, "");
static_assert(SPOOPY_OPTIMIZATION_LEVEL_HIGH == (int)SLANG_OPTIMIZATION_LEVEL_HIGH, "");
static_assert(SPOOPY_OPTIMIZATION_LEVEL_MAXIMAL == (int)SLANG_OPTIMIZATION_LEVEL_MAXIMAL, "");

static_assert(SPOOPY_STAGE_INVALID == (int)SLANG_STAGE_NONE, "");
static_assert(SPOOPY_STAGE_VERTEX == (int)SLANG_STAGE_VERTEX, "");
static_assert(SPOOPY_STAGE_FRAGMENT == (int)SLANG_STAGE_FRAGMENT, "");

struct spoopy_context {
    SlangSession* session;
    SlangCompileRequest* compile_request;
};

spoopy_context_t global_context_pool = {0};

bool spoopy_global_context_init() {
    SlangResult result = slang_createGlobalSession(SLANG_API_VERSION, &global_context_pool.session);
    if(SLANG_FAILED(result)) {
        SPOOPY_LOG_ERROR("Failed to create global Slang session: %d", result);
        return false;
    }

    return true;
}

void spoopy_shader_cleanup() {
    if (global_context_pool.session) {
        global_context_pool.session->release();
        global_context_pool.session = NULL;
    }
    global_context_pool.compile_request = NULL;
}

bool spoopy_api_shader_supported(spoopy_transpile_options_t* transpile_opts, const spoopy_shader_info_t* info) {
    using namespace slang;

    uint32_t family = 0;

    switch(info->target) {
        case SLANG_SPIRV:
        case SLANG_SPIRV_ASM:
        case SLANG_HLSL:
        case SLANG_DXBC:
        case SLANG_DXBC_ASM:
        case SLANG_DXIL:
        case SLANG_DXIL_ASM:
        case SLANG_METAL:
        case SLANG_METAL_LIB:
        case SLANG_METAL_LIB_ASM:
            break;
        default:
            return false;
    }

#if defined(KORE_METAL)
    family  |= (1 << SLANG_METAL)
            |  (1 << SLANG_METAL_LIB)
            |  (1 << SLANG_METAL_LIB_ASM);
#elif defined(KORE_DIRECT3D11)
    family  |= (1 << SLANG_DXBC)
            |  (1 << SLANG_DXBC_ASM)
            |  (1 << SLANG_DXIL)
            |  (1 << SLANG_DXIL_ASM)
            |  (1 << SLANG_HLSL);
#elif defined(KORE_DIRECT3D12)
    family  |= (1 << SLANG_DXIL)
            |  (1 << SLANG_DXIL_ASM)
            |  (1 << SLANG_HLSL);
#elif defined(KORE_VULKAN)
    family  |= (1 << SLANG_SPIRV)
            |  (1 << SLANG_SPIRV_ASM)
#endif

    if(family &= (1 << info->target)) {
        return true;
    }

    if(transpile_opts) {
        transpile_opts->compile.optimization_level = SPOOPY_OPTIMIZATION_LEVEL_DEFAULT;
        transpile_opts->decompile.target = info->target;
    }

    return false;
}
