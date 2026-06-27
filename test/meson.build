# Only build tests if the option is enabled
opt_tests = get_option('enable_tests')

if opt_tests.disabled()
    subdir_done()
endif

test_inc = include_directories('.')
test_links = []

tests = [
    'triangle',
    'texture',
    'cube',
    'audio',
    'ui-test',
]

if host_machine.system() != 'emscripten'
    tests += ['parallel']
endif

foreach test : tests
    test_name = test
    test_deps = [spoopy_deps]
    test_inc  = [test_inc, spoopy_includes]
    test_link_args = []

    if test == 'ui-test'
		subdir('ui')
		test_deps += cimgui_dep
		if host_machine.system() == 'darwin'
			test_link_args += ['-target', 'arm64-apple-darwin', macos_min_arg, '-lc++']
		endif
    endif

    executable(test_name,
        '@0@.c'.format(test),
        dependencies: test_deps, # TODO (Framework): Can't have deps for the framework portion
        link_with: [spoopy_core_lib],
        link_language: 'c',
        link_args: test_link_args,
        include_directories: test_inc,
        install: false,
    )
endforeach
