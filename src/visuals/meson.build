# MSRV - Minimum Supported Rust Version
# If you change this, please update these:
#   - the "rust-version" value in Cargo.toml
msrv = '1.88.0'
cargo = find_program('cargo', version:'>= @0@'.format(msrv))
cargo_c = find_program('cargo-cbuild', version:'>=0.10.8')
rust_c = find_program('rustc', version:'>= @0@'.format(msrv))

if host_machine.system() == 'windows'
    lib_prefix = cc.get_argument_syntax() == 'msvc' ? '' : 'lib'
    ext_static = cc.get_argument_syntax() == 'msvc' ? 'lib' : 'a'
else
    lib_prefix = 'lib'
    ext_static = 'a'
endif

cargo_toml = meson.current_source_dir() / 'Cargo.toml'

cargo_wrapper_args = [
    '--command', 'cbuild',
    '--cargo', cargo.full_path(),
    '--manifest-path', cargo_toml,
    '--project-build-root', meson.project_build_root(),
    '--prefix', get_option('prefix'),
    '--libdir', get_option('libdir'),
]

if is_debug_build
    optimization = get_option('optimization')

    if optimization in ['0', '1', '2', '3', 's']
        cargo_wrapper_args += ['--optimization', optimization]
    elif optimization != 'plain'
        cargo_wrapper_args += ['--optimization', '1']
    endif
else
    cargo_wrapper_args += ['--release']
endif

spoopy_visuals_src = files(
    'Cargo.toml',
    'src/lib.rs',
)

spoopy_visuals_stuff = custom_target('spoopy_visuals_lib',
    output: ['@0@spoopy_visuals.@1@'.format(lib_prefix, ext_static)],
    build_by_default: true,
    console: true,
    depend_files: spoopy_visuals_src,
    command: [
        cargo_wrapper
    ] + cargo_wrapper_args,
    install: false,
)

spoopy_visuals_lib = spoopy_visuals_stuff[0]

spoopy_visuals_dep = declare_dependency(
    link_with: spoopy_visuals_lib,
    include_directories: spoopy_includes,
    dependencies: spoopy_src_deps,
)
