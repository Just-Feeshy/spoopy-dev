project('spoopy_renderer', ['c', 'cpp'],
  meson_version: '>= 0.63.0',
      default_options: run_command([
          files('scripts/default_options.py'),
          '--language', 'c++',
          '--build-type', 'auto',
      ], check: true).stdout().strip().split('\n')
)

# Get filesystem module and parent project's include directories
fs = import('fs')
spoopy_includes = include_directories('../../include')

spoopy_dep_kore = dependency('kore', fallback : ['kore-wrapper', 'kore_dep'])

config_renderer = configuration_data()
opt_kore = get_option('enable_kore')

if opt_kore.enabled()
    config_renderer.set('SPOOPY_BACKEND_INCLUDE', 'backend/kore/')
endif

# Check if we have kore video source file
kore_video_src = []
if fs.exists('backend/kore/spoopy_video.c')
    kore_video_src = files('backend/kore/spoopy_video.c')
endif

spoopy_renderer_lib = static_library('spoopy_renderer',
    sources: kore_video_src,
    include_directories: [spoopy_includes],
    dependencies: [spoopy_dep_kore],
    install: false
)

configure_file(
    configuration: config_renderer,
    output: 'spoopy_renderer_config.h'
)

spoopy_renderer_dep = declare_dependency(
    link_with: spoopy_renderer_lib,
    include_directories: [spoopy_includes],
    dependencies: [spoopy_dep_kore]
)
