#include <slang-com-ptr.h>
#include <slang.h>
#include <spoopy_log.h>
#include <spoopy_context.h>

struct spoopy_context {
    SlangSession* session;
    SlangCompileRequest* compile_request;
};

int spoopy_new_context(spoopy_context_t* context) {
    assert(slang_createGlobalSession(SLANG_API_VERSION, &context->session) == SLANG_OK);

    SlangResult result = context->session->createCompileRequest(&context->compile_request);
    if(SLANG_FAILED(result)) {
        SPOOPY_LOG_ERROR("Failed to create compile request for the video device.");
        return 0;
    }

#if defined(KORE_METAL)
    SlangCompileTarget target = SLANG_METAL;
#elif defined(KORE_VULKAN)
    SlangCompileTarget target = SLANG_SPIRV;
#else
    SlangCompileTarget target = SLANG_HLSL;
#endif

    int target_index = context->compile_request->addCodeGenTarget(target);
    auto profile = context->session->findProfile("spirv");
    context->compile_request->setTargetProfile(target_index, profile);

    return 1;
}
