is_msvc_style = cc.get_argument_syntax() == 'msvc' or cc.get_define('_MSC_VER') != ''

# Cross-platform warning arguments
if is_msvc_style

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

spoopy_src_deps = []

if host_machine.system() == 'windows'
    spoopy_src_deps += [cc.find_library('shlwapi')]
endif

dep_m = cc.find_library('m', required: false)
if dep_m.found()
    spoopy_src_deps += dep_m
endif

spoopy_core_src = []

# Add build directory for generated headers (spoopy_system_info.h)
spoopy_includes += include_directories(meson.current_build_dir())

# subdir('renderer')
subdir('core')
# subdir('visuals')

configure_file(
    configuration: config,
    output: 'spoopy_system_info.h'
)
