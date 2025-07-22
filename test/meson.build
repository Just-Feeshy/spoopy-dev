# Only build tests if the option is enabled
opt_tests = get_option('tests')

if opt_tests.disabled()
    subdir_done()
endif

test_inc = include_directories('.')

tests = [
    'parallel',
]

foreach test : tests
    executable(test,
        '@0@.c'.format(test),
        dependencies: spoopy_deps,
        link_with: spoopy_core_lib,
        include_directories: [test_inc, spoopy_includes],
        install: false,
    )

    # test(test, test_exe)
endforeach
