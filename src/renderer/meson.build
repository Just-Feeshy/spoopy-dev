project('spoopy_renderer', ['c', 'cpp'],
  meson_version: '>= 0.63.0',
      default_options: run_command([
          files('../../scripts/default_options.py'),
          '--language', 'c++',
          '--build-type', 'auto',
      ], check: true).stdout().strip().split('\n')
)

spoopy_dep_kore = dependency('kore', fallback : ['kore-wrapper', 'kore_dep'])

config_renderer = configuration_data()
opt_kore = get_option('enable_kore')

if opt_kore.enabled()
    config_renderer.set('SPOOPY_BACKEND_INCLUDE', 'backend/kore/')
endif

spoopy_renderer_src = files(
    'spoopy_video.c'
)

spoopy_renderer_lib = static_library('spoopy_renderer',
    sources: spoopy_renderer_src,
    include_directories: [spoopy_includes],
    dependencies: [spoopy_dep_kore] + spoopy_deps,
    c_args: spoopy_c_args,
    c_pch: 'pch/spoopy_pch.h',
    install: false
)

configure_file(
    configuration: config_renderer,
    output: 'spoopy_renderer_config.h'
)

spoopy_renderer_dep = declare_dependency(
    link_with: spoopy_renderer_lib,
    include_directories: [spoopy_includes],
    dependencies: [spoopy_dep_kore] + spoopy_deps
)
