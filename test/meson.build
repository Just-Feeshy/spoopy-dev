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


# Only testing gets access to SDL

foreach test : tests
    test_deps = [spoopy_deps] + [spoopy_sdl_deps]
    test_inc = [test_inc, spoopy_includes]

    if opt_kore.enabled()
        test_inc += [kore_inc_dirs]
    endif

    executable(test,
        '@0@.c'.format(test),
        dependencies: test_deps,
        link_with: [spoopy_core_lib, spoopy_renderer],
        include_directories: test_inc,
        install: false,
    )
endforeach
