spoopy_core_src += files(
    'spoopy_api.c',
    'spoopy_core_thread.c',
	'spoopy_image.c',
    'utils/spoopy_misc_math.c',
	'format/spoopy_png.c',
)

subdir('sdl')
subdir('memory')

spoopy_core_src += [
	spoopy_sdl_src,
	spoopy_memory_src,
]

spoopy_core_deps = [
	spoopy_deps,
	spoopy_src_deps,
	spoopy_sdl_deps,
	spoopy_memory_deps,
]

if host_machine.system() == 'emscripten'
    spoopy_core_lib = static_library('spoopy',
        sources: [spoopy_core_src],
        include_directories: [spoopy_includes],
        dependencies: spoopy_core_deps,
        link_with: [spoopy_renderer],
        c_args: spoopy_c_args,
        c_pch: 'pch/spoopy_pch.h',
        install: false
    )
else
    spoopy_core_lib = shared_library('spoopy',
        sources: [spoopy_core_src],
        include_directories: [spoopy_includes],
        dependencies: spoopy_core_deps,
        link_with: [spoopy_renderer],
        c_args: spoopy_c_args,
        c_pch: 'pch/spoopy_pch.h',
        version: meson.project_version(),
        gnu_symbol_visibility: 'default',
        install: false,
        soversion: '0'
    )
endif
