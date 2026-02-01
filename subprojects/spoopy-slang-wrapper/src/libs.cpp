#define SPOOPY_NO_HEADER_SLANG

#include <slang.h>
#include <slang-com-ptr.h>
#include <slang-com-helper.h>

#include <spoopy_log.h>
#include <spoopy_shader.h>
#include <spoopy_graphics.h>
#include <memory/spoopy_memory.h>

#include <cstdint>
#include <initializer_list>
#include <cstring>

#include <bx/string.h>
#include <tinystl/string.h>
#include <tinystl/vector.h>

using namespace slang;

struct target_profile {
	SlangCompileTarget target = SLANG_TARGET_UNKNOWN;
	SlangProfileID     profile = SLANG_PROFILE_UNKNOWN;
};

static inline SlangProfileID try_find_profile(slang::IGlobalSession* globalSession, std::initializer_list<const char*> candidates) {
	for (const char* name : candidates) {
		if(!name) {
			continue;
		}

		SlangProfileID id = globalSession->findProfile(name);
		if(id != SLANG_PROFILE_UNKNOWN) {
			return id;
		}
	}

	return SLANG_PROFILE_UNKNOWN;
}

static inline target_profile pick_target_profile(slang::IGlobalSession* globalSession, spoopy_renderer_t renderer_mask) {
	target_profile out{};

    spoopy_renderer_t renderer = spoopy_graphics_pick_renderer(renderer_mask);
    if(!spoopy_graphics_renderer_supported(renderer)) {
        return out;
	}

	switch(renderer) {
		case SPOOPY_RENDERER_API_D3D11: {
			out.target  = SLANG_DXBC;
			out.profile = try_find_profile(globalSession, { "sm_5_0", "sm_5_1" });

			return out;
		}
		case SPOOPY_RENDERER_API_METAL: {
			out.target = SLANG_METAL;
			out.profile = try_find_profile(globalSession, {
				"metal",
				"metal_3_0", "metal_2_4", "metal_2_3", "metal_2_2", "metal_2_1", "metal_2_0"
			});

			return out;
		}
		case SPOOPY_RENDERER_API_WGPU: {
			out.target = SLANG_WGSL;
			out.profile = try_find_profile(globalSession, { "wgsl", "wgsl_1_0" });

			return out;
		}
		default: {
			break;
		}
	}

	return out;
}

static inline void configure_target_desc(slang::TargetDesc& td, SlangCompileTarget target) {
	td.lineDirectiveMode = SLANG_LINE_DIRECTIVE_MODE_STANDARD;
	td.flags = 0;

	switch (target) {
		case SLANG_METAL:
			td.lineDirectiveMode = SLANG_LINE_DIRECTIVE_MODE_NONE;
			break;
		default:
			break;
	}
}

static inline bool char_is_space(char ch) {
    switch (ch) {
        case ' ':  case '\t':
        case '\n': case '\r':
        case '\v': case '\f':
            return true;
        default:
            return false;
    }
}

static inline bool char_is_digit(char ch) {
    return ch >= '0' && ch <= '9';
}

static inline bool char_is_alpha(char ch) {
    return (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z');
}

static inline bool char_is_alphanum(char ch) {
    return char_is_alpha(ch) || char_is_digit(ch);
}


// Those that know me personally, I REALLY don't like the C++ style of programming.
extern "C" {

static_assert(SPOOPY_OPTIMIZATION_LEVEL_NONE == (int)SLANG_OPTIMIZATION_LEVEL_NONE, "");
static_assert(SPOOPY_OPTIMIZATION_LEVEL_DEFAULT == (int)SLANG_OPTIMIZATION_LEVEL_DEFAULT, "");
static_assert(SPOOPY_OPTIMIZATION_LEVEL_HIGH == (int)SLANG_OPTIMIZATION_LEVEL_HIGH, "");
static_assert(SPOOPY_OPTIMIZATION_LEVEL_MAXIMAL == (int)SLANG_OPTIMIZATION_LEVEL_MAXIMAL, "");

const char* desired_slang_pf = NULL;

struct spoopy_context {
	Slang::ComPtr<slang::IGlobalSession> globalSession;
};

spoopy_context_t global_context;

bool spoopy_global_context_init(void) {
    SlangGlobalSessionDesc desc = {};
    desc.structureSize = sizeof(SlangGlobalSessionDesc);
    desc.apiVersion = SLANG_API_VERSION;
    desc.minLanguageVersion = SLANG_LANGUAGE_VERSION_2025;
    desc.enableGLSL = false;

    SlangResult result = createGlobalSession(&desc, global_context.globalSession.writeRef());
    if(SLANG_FAILED(result) || !global_context.globalSession) {
        SPOOPY_LOG_ERROR("Failed to create global Slang session: %d", result);
        return false;
    }

    return true;
}

void spoopy_shader_cleanup() {
    global_context.globalSession = nullptr;
}


typedef tinystl::basic_string<tinystl::allocator> tiny_string;
typedef tinystl::vector<tiny_string> tiny_string_list;
typedef tinystl::vector<slang::CompilerOptionEntry> compiler_option_list;

static const char* find_substring(const char* start, const char* end, const char* keyword, size_t len) {
    const char* cursor = start;
    while (cursor + len <= end) {
        if (0 == memcmp(cursor, keyword, len)) {
            return cursor;
        }
        ++cursor;
    }
    return end;
}

static bool name_exists(const tiny_string_list& list, const char* data, size_t len) {
    for (size_t i = 0; i < list.size(); ++i) {
        const tiny_string& existing = list[i];
        if (existing.size() == len && 0 == memcmp(existing.c_str(), data, len)) {
            return true;
        }
    }
    return false;
}

static void collect_resource_names(const char* src, size_t len, tiny_string_list& out) {
    if (src == NULL || len == 0) {
        out.clear();
        return;
    }

    out.clear();

    const int32_t clamped_len = (len > (size_t)INT32_MAX) ? INT32_MAX : (int32_t)len;
    bx::StringView haystack(src, clamped_len);
    const char* hay_start = haystack.getPtr();
    const char* hay_end = haystack.getTerm();

    auto scan_keyword = [&](const char* keyword) {
        const size_t key_len = strlen(keyword);
        const char* cursor = hay_start;

        while (cursor < hay_end) {
            const char* found = find_substring(cursor, hay_end, keyword, key_len);
            if (found == hay_end) {
                break;
            }

            const char* ptr = found + key_len;

            while (ptr < hay_end && !char_is_space(*ptr)) {
                ++ptr;
            }
            while (ptr < hay_end && char_is_space(*ptr)) {
                ++ptr;
            }

            const char* name_start = ptr;
            while (ptr < hay_end && (char_is_alphanum(*ptr) || *ptr == '_')) {
                ++ptr;
            }

            if (ptr > name_start && !name_exists(out, name_start, (size_t)(ptr - name_start))) {
                out.push_back(tiny_string(name_start, (size_t)(ptr - name_start)));
            }

            cursor = ptr;
        }
    };

    scan_keyword("Texture");
    scan_keyword("Sampler");
}

static size_t strip_suffixes(const tiny_string& name, char* text, size_t length) {
    if (name.size() == 0 || text == NULL) {
        return length;
    }

    const size_t name_len = name.size();
    char* cursor = text;
    char* end = text + length;

    while (cursor + name_len < end) {
        if (0 == memcmp(cursor, name.c_str(), name_len) && cursor[name_len] == '_') {
            char* digits = cursor + name_len + 1;
            bool has_digits = false;
            while (digits < end && char_is_digit(*digits)) {
                has_digits = true;
                ++digits;
            }

            if (has_digits) {
                const size_t remove_len = (size_t)(digits - (cursor + name_len));
                memmove(cursor + name_len, digits, (size_t)(end - digits));
                end -= remove_len;
                length -= remove_len;
                *end = '\0';
                continue;
            }
        }

        ++cursor;
    }

    return length;
}

static size_t normalize_resource_names(const tiny_string_list& list, char* text, size_t length) {
    if (text == NULL) {
        return length;
    }

    for (size_t i = 0; i < list.size(); ++i) {
        length = strip_suffixes(list[i], text, length);
    }

    return length;
}

static inline void add_int_compiler_option(compiler_option_list& opts, slang::CompilerOptionName name, int32_t value) {
	slang::CompilerOptionEntry e = {};
    e.name = name;
    e.value.kind = slang::CompilerOptionValueKind::Int;
    e.value.intValue0 = value;
    e.value.intValue1 = 0;
    e.value.stringValue0 = nullptr;
    e.value.stringValue1 = nullptr;
    opts.push_back(e);
}

bool spoopy_api_shader_transpile(
    spoopy_shader_source_t* source,
    spoopy_shader_source_t* target,
    spoopy_transpile_options_t* transpile_opts
) {
    if(!source || !target || !transpile_opts) return false;
    if(!global_context.globalSession) {
        if(!spoopy_global_context_init()) {
            SPOOPY_LOG_ERROR("Slang global context is not initialized.");
            return false;
        }
    }

    auto log_diags = [](const char* prefix, slang::IBlob* blob) {
        if(!blob) return;
        const char* s = (const char*)blob->getBufferPointer();
        if(s && *s) {
            SPOOPY_LOG_ERROR("%s%s", prefix, s);
        }
    };

    SlangResult result = SLANG_OK;

    SessionDesc sessionDesc = {};
	target_profile tp = pick_target_profile(global_context.globalSession.get(), source->target);

	if(tp.target == SLANG_TARGET_UNKNOWN) {
		SPOOPY_LOG_ERROR("Unsupported target. renderer=%d", (int)source->target);
		return false;
	}
	if(tp.profile == SLANG_PROFILE_UNKNOWN) {
		SPOOPY_LOG_WARN("No Slang profile found for renderer=%d; continuing with default.", (int)source->target);
	}

	TargetDesc targetDesc = {};
	targetDesc.format  = tp.target;
	targetDesc.profile = tp.profile;

	configure_target_desc(targetDesc, tp.target);
	SlangCompileTarget mapped = tp.target;

	compiler_option_list compilerOptions;
	add_int_compiler_option(compilerOptions, slang::CompilerOptionName::NoMangle, 1);

	if(mapped != SLANG_METAL) {
		add_int_compiler_option(compilerOptions, slang::CompilerOptionName::GenerateWholeProgram, 1);
		add_int_compiler_option(compilerOptions, slang::CompilerOptionName::PreserveParameters, 1);
	}

    targetDesc.compilerOptionEntries = compilerOptions.data();
    targetDesc.compilerOptionEntryCount = (uint32_t)compilerOptions.size();

    sessionDesc.targets = &targetDesc;
    sessionDesc.targetCount = 1;
    sessionDesc.defaultMatrixLayoutMode = SLANG_MATRIX_LAYOUT_COLUMN_MAJOR;
    sessionDesc.allowGLSLSyntax = true;

    Slang::ComPtr<ISession> session;
    result = global_context.globalSession->createSession(sessionDesc, session.writeRef());
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

        size_t normalized_size = size;
        if (mapped == SLANG_METAL) {
            tiny_string_list resources;
            collect_resource_names(source->content, source->content_size, resources);
            normalized_size = normalize_resource_names(resources, buffer, size);
        }

        SPOOPY_LOG_INFO("Content:\n%s\n", buffer);

        target->content_size = normalized_size;
        target->content = (char*)spoopy_heap_alloc(target->content_size + 1);
        if (target->content == NULL) {
            SPOOPY_LOG_ERROR("Out of memory allocating normalized shader source.");
            spoopy_heap_free(buffer);
            return false;
        }
        memcpy((char*)target->content, buffer, target->content_size);
        ((char*)target->content)[target->content_size] = '\0';
        spoopy_heap_free(buffer);

        target->stage = source->stage;
		target->target      = source->target;
        target->entry_point = source->entry_point;
        target->module_name = source->module_name;
		target->entry_point = source->entry_point;
		target->module_name = source->module_name;
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
