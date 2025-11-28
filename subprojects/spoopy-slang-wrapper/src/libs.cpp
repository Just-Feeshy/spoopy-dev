#define SPOOPY_NO_HEADER_SLANG

#include <slang-com-ptr.h>
#include <slang.h>
#include <spoopy_log.h>
#include <spoopy_shader.h>
#include <memory/spoopy_memory.h>

// TODO (Framework) - Remove C++ STL dependency, and use `bx`
#include <vector>

using namespace slang;

// Those that know me personally, I REALLY don't like the C++ style of programming.
extern "C" {

static const int8_t slang_target_mapping[] = {
    SLANG_SPIRV,           // 0 -> SLANG_SPIRV
    SLANG_SPIRV_ASM,       // 1 -> SLANG_SPIRV_ASM
    SLANG_HLSL,            // 2 -> SLANG_HLSL
    SLANG_DXBC,            // 3 -> SLANG_DXBC
    SLANG_DXBC_ASM,        // 4 -> SLANG_DXBC_ASM
    SLANG_DXIL,            // 5 -> SLANG_DXIL
    SLANG_DXIL_ASM,        // 6 -> SLANG_DXIL_ASM
    SLANG_METAL            // 7 -> SLANG_METAL
};

uint32_t spoopy_slang_family = 0;
const char* desired_slang_pf = NULL;

static_assert(SPOOPY_OPTIMIZATION_LEVEL_NONE == (int)SLANG_OPTIMIZATION_LEVEL_NONE, "");
static_assert(SPOOPY_OPTIMIZATION_LEVEL_DEFAULT == (int)SLANG_OPTIMIZATION_LEVEL_DEFAULT, "");
static_assert(SPOOPY_OPTIMIZATION_LEVEL_HIGH == (int)SLANG_OPTIMIZATION_LEVEL_HIGH, "");
static_assert(SPOOPY_OPTIMIZATION_LEVEL_MAXIMAL == (int)SLANG_OPTIMIZATION_LEVEL_MAXIMAL, "");

static_assert(SPOOPY_STAGE_INVALID == (int)SLANG_STAGE_NONE, "");
static_assert(SPOOPY_STAGE_VERTEX == (int)SLANG_STAGE_VERTEX, "");
static_assert(SPOOPY_STAGE_FRAGMENT == (int)SLANG_STAGE_FRAGMENT, "");

struct spoopy_context {
    Slang::ComPtr<SlangSession> session;
};

spoopy_context_t global_context = {0};

bool spoopy_global_context_init() {
    SlangGlobalSessionDesc desc = {};
    desc.structureSize = sizeof(SlangGlobalSessionDesc);
    desc.apiVersion = SLANG_API_VERSION;
    desc.minLanguageVersion = SLANG_LANGUAGE_VERSION_2025;
    desc.enableGLSL = false;

    SlangResult result = createGlobalSession(&desc, global_context.session.writeRef());
    if(SLANG_FAILED(result)) {
        SPOOPY_LOG_ERROR("Failed to create global Slang session: %d", result);
        return false;
    }

    return true;
}

void spoopy_shader_cleanup() {
    global_context.session = nullptr;
}

bool spoopy_api_shader_supported(spoopy_transpile_options_t* transpile_opts, const spoopy_shader_lang_t* info) {
    SPOOPY_LOG_INFO("Checking shader support - desired_slang_pf: %s, target: %d, family: %d",
                    desired_slang_pf ? desired_slang_pf : "(null)", info->target, spoopy_slang_family);

    if(desired_slang_pf && transpile_opts) {
        transpile_opts->profile = desired_slang_pf;
        transpile_opts->target = info->target;
        SPOOPY_LOG_INFO("Set transpile profile to: %s", desired_slang_pf);
    }

    return spoopy_slang_family & (1 << info->target);
}

bool spoopy_api_shader_transpile(
    spoopy_shader_source_t* source,
    spoopy_shader_source_t* target,
    spoopy_transpile_options_t* transpile_opts
) {
    if(!source || !target || !transpile_opts) return false;

    auto log_diags = [](const char* prefix, slang::IBlob* blob) {
        if(!blob) return;
        const char* s = (const char*)blob->getBufferPointer();
        if(s && *s) {
            SPOOPY_LOG_ERROR("%s%s", prefix, s);
        }
    };

    SlangResult result = SLANG_OK;

    SessionDesc sessionDesc = {};
    TargetDesc targetDesc = {};

    const int8_t mapped = slang_target_mapping[transpile_opts->target];
    targetDesc.format = (SlangCompileTarget)mapped;
    targetDesc.profile = global_context.session->findProfile(transpile_opts->profile);
    targetDesc.lineDirectiveMode = SLANG_LINE_DIRECTIVE_MODE_STANDARD;

    switch(mapped) {
        case SLANG_SPIRV:
        case SLANG_SPIRV_ASM:
            targetDesc.flags |= SLANG_TARGET_FLAG_GENERATE_SPIRV_DIRECTLY;
            break;
        case SLANG_METAL:
            // Metal doesn't like whole-program hoisted resource params.
            targetDesc.lineDirectiveMode = SLANG_LINE_DIRECTIVE_MODE_NONE;
            break;
        default:
            targetDesc.flags = 0;
            break;
    }

    // Compiler options
    // Keep NoMangle if you want stable names.
    // IMPORTANT: For Metal, avoid whole-program & parameter-preserve options so Slang
    // doesn't hoist entryPointParams_* to program scope.
    std::vector<slang::CompilerOptionEntry> compilerOptions;
    compilerOptions.push_back({
        slang::CompilerOptionName::NoMangle,
        { slang::CompilerOptionValueKind::Int, 1, 0, nullptr, nullptr }
    });

    if(mapped != SLANG_METAL) {
        compilerOptions.push_back({
            slang::CompilerOptionName::GenerateWholeProgram,
            { slang::CompilerOptionValueKind::Int, 1, 0, nullptr, nullptr }
        });
        compilerOptions.push_back({
            slang::CompilerOptionName::PreserveParameters,
            { slang::CompilerOptionValueKind::Int, 1, 0, nullptr, nullptr }
        });
    }

    targetDesc.compilerOptionEntries = compilerOptions.data();
    targetDesc.compilerOptionEntryCount = (uint32_t)compilerOptions.size();

    sessionDesc.targets = &targetDesc;
    sessionDesc.targetCount = 1;
    sessionDesc.defaultMatrixLayoutMode = SLANG_MATRIX_LAYOUT_COLUMN_MAJOR;
    sessionDesc.allowGLSLSyntax = true;

    Slang::ComPtr<ISession> session;
    result = global_context.session->createSession(sessionDesc, session.writeRef());
    if(SLANG_FAILED(result)) {
        SPOOPY_LOG_ERROR("Failed to create Slang session: %d", result);
        return false;
    }

    // ---- Load embedded module from string ----
    Slang::ComPtr<IModule> module;
    Slang::ComPtr<IBlob> moduleDiags;

    const char* moduleName = source->module_name ? source->module_name : "embedded_shader";
    const char* pathForErrors = transpile_opts->filename ? transpile_opts->filename : "embedded.slang";

    module = session->loadModuleFromSourceString(
        moduleName,
        pathForErrors,
        source->content,
        moduleDiags.writeRef()
    );

    log_diags("Slang module diagnostics:\n", moduleDiags.get());
    if(!module) {
        SPOOPY_LOG_ERROR("Failed to load/compile embedded Slang module.");
        return false;
    }

    // ---- Find entry point ----
    Slang::ComPtr<IEntryPoint> entryPoint;
    {
        Slang::ComPtr<IBlob> epDiags;
        result = module->findEntryPointByName(source->entry_point, entryPoint.writeRef());
        // findEntryPointByName doesn't always produce diags; keep simple.
        (void)epDiags;
    }
    if(!entryPoint) {
        SPOOPY_LOG_ERROR("Failed to find entry point: %s", source->entry_point);
        return false;
    }

    // ---- Compose (module + entry point) ----
    Slang::ComPtr<IComponentType> composed;
    {
        slang::IComponentType* components[] = { module.get(), entryPoint.get() };
        Slang::ComPtr<IBlob> compDiags;
        result = session->createCompositeComponentType(
            components,
            2,
            composed.writeRef(),
            compDiags.writeRef()
        );
        log_diags("Slang composite diagnostics:\n", compDiags.get());
        if(SLANG_FAILED(result) || !composed) {
            SPOOPY_LOG_ERROR("Failed to create composite component type.");
            return false;
        }
    }

    // ---- Link ----
    Slang::ComPtr<IComponentType> linked;
    {
        Slang::ComPtr<IBlob> linkDiags;
        result = composed->link(linked.writeRef(), linkDiags.writeRef());
        log_diags("Slang link diagnostics:\n", linkDiags.get());
        if(SLANG_FAILED(result) || !linked) {
            SPOOPY_LOG_ERROR("Failed to link Slang program.");
            return false;
        }
    }

    // ---- Get ONLY the entry point code (fixes entryPointParams_* globals on Metal) ----
    Slang::ComPtr<IBlob> codeBlob;
    {
        Slang::ComPtr<IBlob> codeDiags;
        const int entryPointIndex = 0; // we composed exactly one entry point
        const int targetIndex = 0;     // we have exactly one target
        result = linked->getEntryPointCode(
            entryPointIndex,
            targetIndex,
            codeBlob.writeRef(),
            codeDiags.writeRef()
        );
        log_diags("Slang codegen diagnostics:\n", codeDiags.get());
        if(SLANG_FAILED(result) || !codeBlob) {
            SPOOPY_LOG_ERROR("Failed to generate target code for entry point.");
            return false;
        }
    }

    // ---- Copy output into target ----
    {
        size_t size = codeBlob->getBufferSize();
        char* buffer = (char*)spoopy_heap_alloc(size + 1);
        if(!buffer) {
            SPOOPY_LOG_ERROR("Out of memory allocating transpiled shader.");
            return false;
        }
        memcpy(buffer, codeBlob->getBufferPointer(), size);
        buffer[size] = '\0';

        SPOOPY_LOG_INFO("Content:\n%s\n", buffer);

        target->content = buffer;
        target->content_size = size;
        target->stage = source->stage;
        target->entry_point = source->entry_point;
        target->module_name = source->module_name;
        target->lang = source->lang;
        target->lang.profile = transpile_opts->profile;
    }

    return true;
}

void spoopy_api_add_macro(spoopy_transpile_options_t* options, const char* name, const char* value) {
    if(!options || !name || !value) {
        SPOOPY_LOG_ERROR("Invalid parameters for adding macros.");
        return;
    }

    options->macros = (spoopy_shader_macro_t*)realloc(options->macros,
        (options->macro_count + 1) * sizeof(spoopy_shader_macro_t)
    );

    options->macros[options->macro_count].name = name;
    options->macros[options->macro_count].value = value;
    options->macro_count++;
}

} // extern "C"

#undef SPOOPY_NO_HEADER_SLANG
