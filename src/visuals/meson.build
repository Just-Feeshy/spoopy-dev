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

cargo_build_args = []

if is_debug_build
    optimization = get_option('optimization')

    if optimization in ['0', '1', '2', '3', 's']
        cargo_build_args += ['--optimization', optimization]
    elif optimization != 'plain'
        cargo_build_args += ['--optimization', '1']
    endif
else
    cargo_build_args += ['--release']
endif

visuals_sources = files(
    'Cargo.toml',
    'src/lib.rs',
)

rust_artifacts = custom_target(
    'spoopy-visuals',
    build_by_default: true,
    output: '@0@spoopy_visuals.@1@'.format(lib_prefix, ext_static),
    console: true,
    depend_files: visuals_sources,
    command: [
        cargo_wrapper,
        '--command', 'cbuild',
        '--cargo', cargo.full_path(),
        '--manifest-path', cargo_toml,
        '--current-build-dir', '@OUTDIR@',
        '--current-source-dir', meson.current_source_dir(),
        '--project-build-root', meson.project_build_root(),
        '--prefix', get_option('prefix'),
        '--libdir', get_option('libdir'),
        '--extension', ext_static,
    ] + cargo_build_args,
    install: false,
)

spoopy_visuals_lib = rust_artifacts

spoopy_visuals_dep = declare_dependency(
    link_with: spoopy_visuals_lib,
    include_directories: spoopy_includes,
    dependencies: spoopy_src_deps,
)
