# Spoopy - A lightweight C game framework
# Build system "inspired" by Taisei Project

# Go check the project out (I'm a Touhou fan):
# https://github.com/taisei-project/taisei


project('spoopy', ['c'],
    license: 'mit',
     meson_version: '>=1.5.2',
    version: run_command([
        'node', '-e', 'console.log(require("./package.json").version)'
    ], check: true).stdout().strip(),
    default_options: run_command([
        files('scripts/default_options.js'),
        '--language', 'c',
        '--build-type', 'auto',
    ], check: true).stdout().strip().split('\n')
)

cc = meson.get_compiler('c')
config = configuration_data()

if host_machine.system() == 'darwin'
    add_project_arguments('-mmacosx-version-min=12.0', language: ['c', 'objc'])
endif

spoopy_includes = include_directories('include', 'include')

is_debug_build = get_option('debug')
deprecation_warnings = get_option('deprecation_warnings')

spoopy_deps = []

spoopy_deps += dependency('sdl3', version: '>=3.2.0', fallback : ['sdl3-cmake-wrapper', 'sdl3_dep'], required: true)

has_posix = cc.has_header_symbol('unistd.h', '_POSIX_VERSION')

if has_posix
    if not cc.has_header_symbol('stdio.h', 'fileno')
        config.set('_POSIX_C_SOURCE', '200809L')
    endif

    if host_machine.system() == 'darwin'
        config.set('_DARWIN_C_SOURCE', true)
    endif
endif

config.set('SPOOPY_HAS_POSIX', has_posix)
config.set('SPOOPY_HAS_ALIGNED_ALLOC', cc.has_function('aligned_alloc'))
config.set('SPOOPY_HAS_POSIX_MEMALIGN',
    cc.has_function('posix_memalign') and host_machine.system() != 'nx')

if host_machine.system() != 'emscripten'
    malloc_alignment = cc.alignment('max_align_t', prefix: '#include <stddef.h>\n')
    config.set('SPOOPY_MALLOC_ALIGNMENT', malloc_alignment)
endif

config.set('SPOOPY_BUILD_DEBUG', is_debug_build)

foreach header : ['stdint.h', 'stdbool.h', 'string.h', 'stdlib.h', 'stdio.h']
    if cc.has_header(header)
        config.set('SPOOPY_HAS_@0@'.format(header.underscorify().to_upper()), 1)
    endif
endforeach

systype = '@0@, @1@, @2@'.format(
    host_machine.system(),
    host_machine.cpu_family(),
    host_machine.cpu()
)

if meson.is_cross_build()
    systype = '@0@ (cross-compiled)'.format(systype)
endif


# Get absolute paths
# "Borrowed" from Sway, also, very confused about what
# this was for, so I wrote comments to explain each process
# for others' planning to "borrow" this code.

source_root = meson.current_source_dir()    # e.g., "/home/user/spoopy"
build_root = meson.project_build_root()     # e.g., "/home/user/spoopy/builddir"

if build_machine.system() == 'windows'
    source_root = source_root.replace('\\', '/')  # C:\project → C:/project
    build_root = build_root.replace('\\', '/')    # C:\project\build → C:/project/build
endif

source_root = source_root.split('/')  # ["/home", "user", "spoopy"]
build_root = build_root.split('/')    # ["/home", "user", "spoopy", "builddir"]

relative_dir_parts = []
i = 0
in_prefix = true
foreach p : build_root
    if i >= source_root.length() or not in_prefix or p != source_root[i]
        in_prefix = false
        relative_dir_parts += '..'
    endif
    i += 1
endforeach
i = 0
in_prefix = true
foreach p : source_root
    if i >= build_root.length() or not in_prefix or build_root[i] != p
        in_prefix = false
        relative_dir_parts += p
    endif
    i += 1
endforeach

relative_dir = join_paths(relative_dir_parts)
relative_src_dir = relative_dir + '/src/'

if cc.has_argument('-fmacro-prefix-map=/prefix/to/hide=')
    add_project_arguments(
        '-fmacro-prefix-map=@0@=.'.format(relative_src_dir),
        language: 'c',
    )
else
    config.set_quoted('SPOOPY_REAL_DIR', relative_dir)
    config.set_quoted('SPOOPY_REAL_SRC_DIR', relative_src_dir)
endif

subdir('scripts')
subdir('src')
subdir('test')

spoopy_sdl_threads_support = get_option('have_sdl_thread_support')

if not spoopy_sdl_threads_support
    config.set('SPOOPY_SUPPORT_SDL_THREADS', 0)
endif

summary({
    'System Type': systype,
    'Build Type': is_debug_build ? 'debug' : 'release',
    'Deprecation Warnings': deprecation_warnings,
    'SDL Threading Support': spoopy_sdl_threads_support ? 'enabled' : 'disabled',
}, section: 'Configuration', bool_yn: true)
