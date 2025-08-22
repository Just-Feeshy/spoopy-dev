#define SPOOPY_NO_HEADER_SLANG

#include <slang-com-ptr.h>
#include <slang.h>
#include <spoopy_log.h>
#include <spoopy_shader.h>
#include <memory/spoopy_memory.h>

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
    SlangResult result = SLANG_OK;

    SessionDesc sessionDesc = {};
    TargetDesc targetDesc = {};
    targetDesc.format = (SlangCompileTarget)slang_target_mapping[transpile_opts->target];
    targetDesc.lineDirectiveMode = SLANG_LINE_DIRECTIVE_MODE_STANDARD;
    targetDesc.profile = global_context.session->findProfile(transpile_opts->profile);

	switch(slang_target_mapping[transpile_opts->target]) {
		case SLANG_SPIRV:
		case SLANG_SPIRV_ASM:
			targetDesc.flags |= SLANG_TARGET_FLAG_GENERATE_SPIRV_DIRECTLY;
			break;
        case SLANG_METAL:
            break;
		default:
			targetDesc.flags = 0;
			break;
	}

    std::vector<slang::CompilerOptionEntry> compilerOptions;
    compilerOptions.push_back({
        slang::CompilerOptionName::NoMangle,
        { slang::CompilerOptionValueKind::Int, 1, 0, nullptr, nullptr }
    });
    compilerOptions.push_back({
        slang::CompilerOptionName::GenerateWholeProgram,
        { slang::CompilerOptionValueKind::Int, 1, 0, nullptr, nullptr }
    });

    targetDesc.compilerOptionEntries = compilerOptions.data();
    targetDesc.compilerOptionEntryCount = static_cast<uint32_t>(compilerOptions.size());

    Slang::ComPtr<ISession> session;
    Slang::ComPtr<IModule> module;
    Slang::ComPtr<IComponentType> program;
    Slang::ComPtr<IComponentType> linkedProgram;
    Slang::ComPtr<IBlob> codeBlob;
    Slang::ComPtr<IBlob> diagnostics_blob;

    sessionDesc.targets = &targetDesc;
    sessionDesc.targetCount = 1;

    sessionDesc.defaultMatrixLayoutMode = SLANG_MATRIX_LAYOUT_COLUMN_MAJOR;
    sessionDesc.allowGLSLSyntax = true;

    result = global_context.session->createSession(sessionDesc, session.writeRef());
    if(SLANG_FAILED(result)) {
        goto slang_fail;
    }

    if(!strcmp(transpile_opts->filename, "<embedded>")) {
        module = session->loadModuleFromSourceString(
            source->module_name ? source->module_name : "embedded_shader",
            NULL,
            source->content,
            NULL
        );
    }
    if(!module) {
        result = SLANG_E_CANNOT_OPEN;
        goto slang_fail;
    }

    result = module->link(linkedProgram.writeRef(), diagnostics_blob.writeRef());
    if(SLANG_FAILED(result)) {
        goto slang_fail;
    }

    result = linkedProgram->getTargetCode(0, codeBlob.writeRef(), diagnostics_blob.writeRef());
    if(SLANG_SUCCEEDED(result) && codeBlob) {
        size_t size = codeBlob->getBufferSize();
        char* buffer = (char*)spoopy_heap_alloc(size + 1);

        if(buffer) {
            memcpy(buffer, codeBlob->getBufferPointer(), size);
            buffer[size] = '\0';

            target->content = buffer;
            target->content_size = size;
            target->stage = source->stage;
            target->entry_point = source->entry_point;
            target->module_name = source->module_name;
            target->lang = source->lang;
        } else {
            result = SLANG_E_OUT_OF_MEMORY;
        }
    }

    if(diagnostics_blob) {
        const char* diagnostics = (const char*)diagnostics_blob->getBufferPointer();
        if(*diagnostics) {
			bool is_spirv = (slang_target_mapping[transpile_opts->target] == SLANG_SPIRV_ASM ||
                slang_target_mapping[transpile_opts->target] == SLANG_SPIRV_ASM);
            bool is_spirv_warning = (strstr(diagnostics, "spirv-opt") ||
                                     strstr(diagnostics, "spirv-dis") ||
                                     strstr(diagnostics, "slang-glslang"));

            if(is_spirv && !is_spirv_warning) {
                SPOOPY_LOG_ERROR("Shader transpilation failed: %s", diagnostics);
            }
        }
    }

slang_fail:
    switch(result) {
        case SLANG_E_NOT_FOUND:
            SPOOPY_LOG_ERROR("Shader target not found: %s", transpile_opts->profile);
            return false;
        case SLANG_E_CANNOT_OPEN:
            SPOOPY_LOG_ERROR("Cannot open shader file: %s", transpile_opts->filename);
            return false;
        default:
            return true;
    }
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
