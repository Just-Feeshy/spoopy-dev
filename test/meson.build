# Only build tests if the option is enabled
opt_tests = get_option('enable_tests')

if opt_tests.disabled()
    subdir_done()
endif

test_inc = include_directories('.')
test_links = []
fukkireta_fastnoiselite = get_option('fastnoiselite')

tests = [
	'null_pipeline',
    'triangle',
	'texture',
	'cube',
	'fukkireta',
]

if host_machine.system() != 'emscripten'
    tests += ['parallel']
endif


# Only testing gets access to SDL

foreach test : tests
    test_deps = [spoopy_deps] + [spoopy_sys_deps] + [spoopy_renderer_deps]
    test_inc = [test_inc, spoopy_includes]
    test_c_args = []

    if test == 'fukkireta' and fukkireta_fastnoiselite
        test_deps += [fastnoiselite_dep]
        test_c_args += ['-DSPOOPY_FUKKIRETA_FASTNOISELITE=1']
    endif

    executable(test,
        '@0@.c'.format(test),
        dependencies: test_deps,
        link_with: [spoopy_core_lib],
        include_directories: test_inc,
        c_args: test_c_args,
        install: false,
    )
endforeach
