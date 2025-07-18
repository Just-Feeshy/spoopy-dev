# Spoopy - A lightweight C game framework
# Build system "inspired" by Taisei Project

# Go check the project out (I'm a Touhou fan):
# https://github.com/taisei-project/taisei


project('spoopy', 'c',
    license: 'mit',
    meson_version: '>=0.63.0',
    version: '0.0.1',
    default_options: run_command([
        files('scripts/default_options.py'),
        '--build-type', 'auto', # TODO: Make this configurable (through custom make.py)
    ], check: true).stdout().strip().split('\n'),
)

cc = meson.get_compiler('c')
config = configuration_data()

# Cross-platform warning arguments
if cc.get_id() == 'msvc'

    # MSVC warning flags
    spoopy_c_args = [
        '/W4',              # High warning level (equivalent to -Wall -Wextra)
        '/we4013',          # Implicit function declaration as error
        '/we4133',          # Incompatible pointer types as error
        '/we4716',          # Function must return a value (equivalent to -Werror=return-type)
        '/w14062',          # Enumerator not handled in switch
        '/w14263',          # Member function doesn't override virtual base class function
        '/w14265',          # Class has virtual functions but destructor not virtual
        '/w14287',          # Unsigned/negative constant mismatch
        '/w14296',          # Expression always true/false
        '/w14311',          # Pointer truncation from type to type
        '/w14545',          # Expression before comma has no effect
        '/w14546',          # Function call before comma missing argument list
        '/w14547',          # Operator before comma has no effect
        '/w14549',          # Operator before comma has no effect
        '/w14555',          # Expression has no effect
        '/w14619',          # pragma warning doesn't exist
        '/w14640',          # Construction of local static object not thread-safe
        '/w14826',          # Conversion from type to type is sign-extended
        '/w14905',          # Wide string literal cast to LPSTR
        '/w14906',          # String literal cast to LPWSTR
        '/w14928',          # Illegal copy-initialization
        '/wd4142',          # Conversion from type to type, possible loss of data
        '/permissive-',     # Disable non-conforming code
        '/volatile:iso',    # ISO volatile semantics
    ]

    # MSVC conversion warnings
    spoopy_c_conversion_args = [
        '/w14244',          # Conversion with possible loss of data
        '/w14242',          # Conversion, possible loss of data (int to char)
        '/w14254',          # Conversion, possible loss of data (loop operator)
        '/w14267',          # Conversion from size_t to smaller type
        '/wd4018',          # Signed/unsigned mismatch (disabled - too noisy)
        '/wd4389',          # Signed/unsigned equality operation (disabled)
        '/wd4245',          # Signed/unsigned conversion (disabled - too noisy)
        '/wd4365',          # Signed/unsigned conversion (disabled - too noisy)
    ]

elif cc.get_id() == 'gcc' or cc.get_id() == 'clang'

    # GCC/Clang warning flags
    spoopy_c_args = [
        '-Wall',
        '-Wextra',
        '-Werror=implicit-function-declaration',
        '-Werror=incompatible-pointer-types',
        '-Werror=return-type',
        '-Wabsolute-value',
        '-Wcast-align',
        '-Wcast-align=strict',
        '-Wclobbered',
        '-Wduplicated-branches',
        '-Wduplicated-cond',
        '-Wfor-loop-analysis',
        '-Wformat-security',
        '-Wgcc-compat',
        '-Wignored-qualifiers',
        '-Winit-self',
        '-Wlogical-op',
        '-Wmissing-prototypes',
        '-Wnull-dereference',
        '-Wparentheses',
        '-Wsometimes-uninitialized',
        '-Wstrict-overflow=0',
        '-Wstrict-prototypes',
        '-Wtype-limits',
        '-Wunneeded-internal-declaration',
        '-Wunreachable-code',
        '-Wunreachable-code-loop-increment',
        '-Wno-typedef-redefinition'
    ]

    # GCC/Clang conversion warnings
    spoopy_c_conversion_args = [
        '-Wconversion',
        '-Wno-enum-conversion',
        '-Wno-float-conversion',
        '-Wfloat-overflow-conversion',
        '-Wno-implicit-int-conversion',
        '-Wno-implicit-int-float-conversion',
        '-Wno-implicit-float-conversion',
        '-Wno-shorten-64-to-32',
        '-Wno-shorten-64-to-32-in-cast',
        '-Wno-sign-conversion',
        '-Wno-string-conversion'
    ]

else

    # Unknown compiler - minimal flags
    spoopy_c_args = []
    spoopy_c_conversion_args = []
    warning('Unknown compiler: @0@, using minimal warning flags'.format(cc.get_id()))

endif

spoopy_includes = include_directories('include', 'include')

is_debug_build = get_option('is_debug')
deprecation_warnings = get_option('deprecation_warnings')

# Cross-platform deprecation warning handling
if cc.get_id() == 'msvc'

    # MSVC deprecation flags
    if deprecation_warnings == 'error'
        spoopy_c_args += '/we4996'        # Treat deprecation as error
    elif deprecation_warnings == 'no-error'
        spoopy_c_args += '/w14996'        # Show deprecation warnings
    elif deprecation_warnings == 'ignore'
        spoopy_c_args += '/wd4996'        # Disable deprecation warnings
    endif

elif cc.get_id() in ['gcc', 'clang']

    # GCC/Clang deprecation flags
    if deprecation_warnings == 'error'
        spoopy_c_args += '-Werror=deprecated-declarations'
    elif deprecation_warnings == 'no-error'
        spoopy_c_args += '-Wno-error=deprecated-declarations'
    elif deprecation_warnings == 'ignore'
        spoopy_c_args += '-Wno-deprecated-declarations'
    endif

else

    # Unknown compiler - show warning
    if deprecation_warnings != 'default'
        warning('Deprecation warning control not supported for compiler: @0@'.format(cc.get_id()))
    endif

endif

spoopy_c_args = cc.get_supported_arguments(spoopy_c_args)

foreach arglist : [
        spoopy_c_conversion_args,
    ]
    if cc.has_multi_arguments(arglist)
        spoopy_c_args += arglist
    endif
endforeach

spoopy_dep_sdl3 = dependency('sdl3', version: '>=3.2.0', fallback : ['sdl3-cmake-wrapper', 'sdl3_dep'])
spoopy_deps = []

if host_machine.system() == 'windows'
    spoopy_deps += [cc.find_library('shlwapi')]
endif

has_posix = cc.has_header_symbol('unistd.h', '_POSIX_VERSION')

if has_posix
    if not cc.has_header_symbol('stdio.h', 'fileno')
        config.set('_POSIX_C_SOURCE', '200809L')
    endif

    # For good measure
    if host_machine.system() == 'darwin'
        config.set('_DARWIN_C_SOURCE', true)
    endif
endif

config.set('SPOOPY_HAS_POSIX', has_posix)
config.set('SPOOPY_HAS_ALIGNED_ALLOC', cc.has_function('aligned_alloc'))

if host_machine.system() != 'emscripten'
    malloc_alignment = cc.alignment('max_align_t', prefix: '#include <stddef.h>\n')
    config.set('SPOOPY_MALLOC_ALIGNMENT', malloc_alignment)
endif

dep_m = cc.find_library('m', required: false)
if dep_m.found()
    spoopy_deps += dep_m
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

# Normalize Windows paths (\ → /)
if build_machine.system() == 'windows'
    source_root = source_root.replace('\\', '/')  # C:\project → C:/project
    build_root = build_root.replace('\\', '/')    # C:\project\build → C:/project/build
endif

# Split into path components
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

# Set the source directory relative to the build directory
if cc.has_argument('-fmacro-prefix-map=/prefix/to/hide=')
    add_project_arguments(
        '-fmacro-prefix-map=@0@=.'.format(relative_src_dir),
        language: 'c',
    )
else
    config.set_quoted('SPOOPY_REAL_DIR', relative_dir)
    config.set_quoted('SPOOPY_REAL_SRC_DIR', relative_src_dir)
endif

spoopy_sdl_threads_support = get_option('sdl_threads_support')

if not spoopy_sdl_threads_support
    config.set('SPOOPY_SUPPORT_SDL_THREADS', 0)
endif

subdir('src')
subdir('test')

summary({
    'System Type': systype,
    'Build Type': is_debug_build ? 'debug' : 'release',
    'SDL3 Version': spoopy_dep_sdl3.version(),
    'Deprecation Warnings': deprecation_warnings,
    'SDL Threading Support': spoopy_sdl_threads_support ? 'enabled' : 'disabled',
}, section: 'Configuration', bool_yn: true)
