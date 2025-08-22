spoopy_core_src += files(
    # 'spoopy_app.c',
    'spoopy_api.c',
    'spoopy_core_thread.c',
    'memory/spoopy_memory.c',
    'sdl/spoopy_sdl_thread.c',
    'sdl/spoopy_sdl_api.c'
)

if host_machine.system() == 'emscripten'
    spoopy_core_lib = static_library('spoopy',
        sources: [spoopy_core_src],
        include_directories: [spoopy_includes],
        dependencies: [spoopy_deps] + [spoopy_src_deps],
        link_with: [spoopy_renderer],
        c_args: spoopy_c_args,
        c_pch: 'pch/spoopy_pch.h',
        install: false
    )
else
    spoopy_core_lib = shared_library('spoopy',
        sources: [spoopy_core_src],
        include_directories: [spoopy_includes],
        dependencies: [spoopy_deps] + [spoopy_src_deps],
        link_with: [spoopy_renderer],
        c_args: spoopy_c_args,
        c_pch: 'pch/spoopy_pch.h',
        version: meson.project_version(),
        gnu_symbol_visibility: 'default',
        install: false,
        soversion: '0'
    )
endif
