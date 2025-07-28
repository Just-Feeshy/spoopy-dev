#include "spoopy_rhi.h"

#include <spoopy_thread.h>
#include <memory/spoopy_memory.h>
#include <spoopy_log.h>
#include <utils/assert.h>

// TODO: Remove the hardcoded backend name
static const char* default_backend_name = "kore";

static const spoopy_renderer_bootstrap_t* bootstraps[] = {
#define R(x) &spoopy_##x##_backend,
    SPOOPY_HAS_RENDERER_BACKENDS
#undef R
    NULL
};

spoopy_renderer_bootstrap_t backend_bootstrap;

static void set_backend_bootstrap(spoopy_renderer_bootstrap_t* bootstrap) {
    memcpy(&backend_bootstrap, bootstrap, sizeof(spoopy_renderer_bootstrap_t));
}

static spoopy_renderer_bootstrap_t* find_bootstrap(const char* name) {
    for (const spoopy_renderer_bootstrap_t** b = bootstraps; *b; ++b) {
        if (!strcmp((*b)->name, name)) {
            return *b;
        }
    }

    SPOOPY_LOG_ERROR("Renderer backend '%s' not found", name);
}

static inline void init_bootstrap(void) {
    static bool initialized;

    if(initialized) {
        SPOOPY_LOG_WARN("Renderer backend initialization requires spoopy threads to be initialized");
        return;
    }

    const char* backend = default_backend_name;
    spoopy_renderer_bootstrap_t* fbts = find_bootstrap(backend);
    fbts->funcs.init();
    set_backend_bootstrap(fbts);

    initialized = true;
}

void spoopy_new_rhi(void) {
    init_bootstrap();
    // Initialize the renderer backend
}
