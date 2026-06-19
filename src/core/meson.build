spoopy_core_src += files(
    'spoopy_api.c',
    'spoopy_core_thread.c',
    'spoopy_framerate.c',
    'spoopy_graphics.c',
	'spoopy_image.c',
	'spoopy_uniform.c',
)

subdir('platform')
subdir('renderer')
subdir('audio')
subdir('depth')
subdir('memory')
subdir('scene')
subdir('utils')

spoopy_core_src += [
	spoopy_audio_src,
	spoopy_depth_src,
	spoopy_memory_src,
	spoopy_platform_src,
	spoopy_renderer_src,
	spoopy_scene_src,
	spoopy_utils_src,
]

spoopy_core_deps = [
	spoopy_deps,
	spoopy_audio_deps,
	spoopy_src_deps,
	spoopy_memory_deps,
	spoopy_renderer_deps,
]

if host_machine.system() == 'emscripten'
    spoopy_core_lib = static_library('spoopy',
        sources: [spoopy_core_src],
        include_directories: [spoopy_includes],
        dependencies: spoopy_core_deps,
        c_args: spoopy_c_args,
        c_pch: 'pch/spoopy_pch.h',
        install: false
    )
else
    spoopy_core_lib = shared_library('spoopy',
        sources: [spoopy_core_src],
        include_directories: [spoopy_includes],
        dependencies: spoopy_core_deps,
        c_args: spoopy_c_args,
        c_pch: 'pch/spoopy_pch.h',
        version: meson.project_version(),
        gnu_symbol_visibility: 'default',
        install: false,
        soversion: '0'
    )
endif
