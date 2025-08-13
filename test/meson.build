# Only build tests if the option is enabled
opt_tests = get_option('enable_tests')

if opt_tests.disabled()
    subdir_done()
endif

test_inc = include_directories('.')
test_links = []

tests = [
    'triangle'
]

if host_machine.system() != 'emscripten'
    tests += ['parallel']
endif

foreach test : tests
    test_deps = spoopy_deps + [slang_lib]

    executable(test,
        '@0@.c'.format(test),
        dependencies: test_deps,
        link_with: [spoopy_core_lib, spoopy_renderer],
        include_directories: [test_inc, spoopy_includes],
        install: false,
    )
endforeach
