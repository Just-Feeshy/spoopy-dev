# Only build tests if the option is enabled
opt_tests = get_option('enable_tests')

if opt_tests.disabled()
    subdir_done()
endif

test_inc = include_directories('.')
test_links = []

tests = [
	'null_pipeline',
    'triangle',
	'texture',
	'cube',
	'audio',
	# 'ui',
	'fukkireta',
]

if host_machine.system() != 'emscripten'
    tests += ['parallel']
endif

foreach test : tests
	# if test == 'ui'
	# 	subdir('ui')
	# endif

    test_deps = [spoopy_deps]
    test_inc  = [test_inc, spoopy_includes]

    executable(test,
        '@0@.c'.format(test),
        dependencies: test_deps, # TODO (Framework): Can't have deps for the framework portion
        link_with: [spoopy_core_lib],
        include_directories: test_inc,
        install: false,
    )
endforeach
