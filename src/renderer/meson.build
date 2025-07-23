opt_kore = get_option('enable_kore')

spoopy_backend_dep = []
spoopy_r_backend_path = ''
spoopy_renderer_src = []

if opt_kore.enabled()
    spoopy_backend_dep = dependency('kore', fallback: ['kore-wrapper', 'kore_dep'])
    spoopy_r_backend_path = join_paths(meson.current_source_dir(), 'backend/kore/')
    spoopy_renderer_src = files(spoopy_r_backend_path + 'spoopy_video.c')
endif

spoopy_renderer_lib = static_library('spoopy_renderer',
    sources: spoopy_renderer_src,
    include_directories: spoopy_includes,
    dependencies: spoopy_backend_dep,
    c_args: spoopy_c_args,
    install: false
)
