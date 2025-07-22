spoopy_core_src = files(
    'spoopy_core_thread.c',
    'memory/spoopy_memory.c',
    'sdl/spoopy_sdl_thread.c'
)

spoopy_core_lib = shared_library('spoopy',
    sources: spoopy_core_src,
    include_directories: [spoopy_includes],
    dependencies: [spoopy_dep_sdl3] + spoopy_deps,
    c_args: spoopy_c_args,
    c_pch: 'pch/spoopy_pch.h',
    version: meson.project_version(),
    gnu_symbol_visibility: 'default',
    install: false,
    soversion: '0'
)
