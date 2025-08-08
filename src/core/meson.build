spoopy_core_src += files(
    # 'spoopy_app.c',
    'spoopy_core_thread.c',
    'memory/spoopy_memory.c',
    'sdl/spoopy_sdl_thread.c'
)

slang_subproject = subproject('slang-cmake-wrapper')
slang_fiddle_gen = slang_subproject.get_variable('slang_fiddle_gen')
slang_cap_gen = slang_subproject.get_variable('slang_cap_gen')

slang_generation_done = custom_target('slang-generation-done',
    command: ['echo', 'Slang generation complete'],
    depends: [slang_fiddle_gen, slang_cap_gen],
    output: 'slang-generation-done.stamp',
    build_always_stale: false
)

if host_machine.system() == 'emscripten'
    spoopy_core_lib = static_library('spoopy',
        sources: [spoopy_core_src, slang_generation_done],
        include_directories: [spoopy_includes],
        dependencies: [spoopy_deps] + [spoopy_src_deps],
        link_with: [spoopy_renderer],
        c_args: spoopy_c_args,
        c_pch: 'pch/spoopy_pch.h',
        install: false
    )
else
    spoopy_core_lib = shared_library('spoopy',
        sources: [spoopy_core_src, slang_generation_done],
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
