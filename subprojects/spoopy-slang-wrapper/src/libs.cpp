#define SPOOPY_NO_HEADER_SLANG

#include <slang-com-ptr.h>
#include <slang.h>
#include <spoopy_log.h>
#include <spoopy_shader.h>

using namespace slang;

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
    SlangResult result = createGlobalSession(SLANG_API_VERSION, &global_context_pool.session);
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

bool spoopy_api_shader_supported(spoopy_transpile_options_t* transpile_opts, const spoopy_shader_lang_t* info) {
    uint32_t family = 0;
    const char* want_profile = NULL;

#if defined(KORE_METAL)
    want_profile = "metallib_2_0";

    family  |= (1 << SLANG_METAL)
            |  (1 << SLANG_METAL_LIB)
            |  (1 << SLANG_METAL_LIB_ASM);
#elif defined(KORE_DIRECT3D11)
    want_profile = "sm_4_0";

    family  |= (1 << SLANG_DXBC)
            |  (1 << SLANG_DXBC_ASM)
            |  (1 << SLANG_DXIL)
            |  (1 << SLANG_DXIL_ASM)
            |  (1 << SLANG_HLSL);
#elif defined(KORE_DIRECT3D12)
    want_profile = "sm_4_0";

    family  |= (1 << SLANG_DXIL)
            |  (1 << SLANG_DXIL_ASM)
            |  (1 << SLANG_HLSL);
#elif defined(KORE_VULKAN)
    want_profile = "spirv_1_0";

    family  |= (1 << SLANG_SPIRV)
            |  (1 << SLANG_SPIRV_ASM)
#endif

    if(!global_context_pool.session->findProfile(want_profile) && transpile_opts) {
        transpile_opts->profile = want_profile;
        transpile_opts->target = info->target;
    }

    if(family &= (1 << info->target)) {
        return true;
    }

    return false;
}

bool spoopy_api_shader_transpile(
    spoopy_shader_source_t* source,
    spoopy_shader_source_t* target,
    spoopy_transpile_options_t* transpile_opts
) {
    SlangResult result = SLANG_OK;

    SessionDesc sessionDesc = {};
    TargetDesc targetDesc = {};
    targetDesc.format = (SlangCompileTarget)transpile_opts->target;
    targetDesc.lineDirectiveMode = SLANG_LINE_DIRECTIVE_MODE_STANDARD;
    targetDesc.profile = global_context_pool.session->findProfile(transpile_opts->profile);

    ISession* session;
    IModule* module = NULL;
    IEntryPoint* entry_point = NULL;
    IComponentType* program = NULL;
    IBlob* codeBlob = NULL;
    IBlob* diagnostics_blob = NULL;
    IComponentType* components[2];

    if(!targetDesc.profile) {
        result = SLANG_E_NOT_FOUND;
        goto slang_fail;
    }

    sessionDesc.targets = &targetDesc;
    sessionDesc.targetCount = 1;

    result  = global_context_pool.session->createSession(sessionDesc, &session);
    if(SLANG_FAILED(result)) {
        goto slang_fail;
    }
    if(!strcmp(transpile_opts->filename, "<embedded>")) {
        module = session->loadModuleFromSourceString(
            source->module_name ? source->module_name : "embedded_shader",
            NULL,
            source->context,
            NULL
        );
    }

    if(!module) {
        result = SLANG_E_CANNOT_OPEN;
        session->release();
        goto slang_fail;
    }

    result = module->findEntryPointByName(source->entry_point, &entry_point);
    if(SLANG_FAILED(result)) {
        session->release();
        goto slang_fail;
    }

    components[0] = module;
    components[1] = entry_point;
    session->createCompositeComponentType(
        components, 2, &program
    );

    result = program->getEntryPointCode(
        0, 0, &codeBlob, &diagnostics_blob
    );

    if(SLANG_SUCCEEDED(result) && codeBlob) {
        target->context = (const char*)codeBlob->getBufferPointer();
        target->context_size = codeBlob->getBufferSize();
        target->stage = source->stage;
        target->entry_point = source->entry_point;
        target->module_name = source->module_name;
    }

    if(diagnostics_blob) {
        const char* diagnostics = (const char*)diagnostics_blob->getBufferPointer();
        if(*diagnostics) {
            SPOOPY_LOG_ERROR("Shader transpilation failed: %s", diagnostics);
        }

        diagnostics_blob->release();
    }

    program->release();
    module->release();
    session->release();

    return true;

slang_fail:
    switch(result) {
        case SLANG_E_NOT_FOUND:
            SPOOPY_LOG_ERROR("Shader target not found: %s", transpile_opts->profile);
            return false;
        case SLANG_E_CANNOT_OPEN:
            SPOOPY_LOG_ERROR("Cannot open shader file: %s", transpile_opts->filename);
            return false;
        default:
            SPOOPY_LOG_ERROR("Unknown error occurred during shader transpilation.");
            return false;
    }
}

void spoopy_api_add_macro(spoopy_transpile_options_t* options, const char* name, const char* value) {
    if(!options || !name || !value) {
        SPOOPY_LOG_ERROR("Invalid parameters for adding macros.");
        return;
    }

    options->macros = (spoopy_shader_macro_t*)realloc(options->macros,
        (options->macro_count + 1) * sizeof(spoopy_shader_macro_t));

    options->macros[options->macro_count].name = name;
    options->macros[options->macro_count].value = value;
    options->macro_count++;
}

#undef SPOOPY_NO_HEADER_SLANG
