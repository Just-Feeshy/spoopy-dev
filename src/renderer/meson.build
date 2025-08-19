opt_kore = get_option('enable_kore')
if opt_kore.enabled()
    config.set('SPOOPY_GPU_IMPL(x)', 'KORE_GPU_IMPL(x)')

    check_submodules_result = run_command(check_submodules_command, check: false)
    if check_submodules_result.stdout() != ''
        error('Failed to check/initialize submodules')
    endif

    kore_dir = meson.current_source_dir() / 'Kore'

    kore_makefile = kore_dir / 'make'
    kore_sources_dir = kore_dir / 'sources'

    if not fs.exists(kore_makefile) or not fs.is_dir(kore_sources_dir)
        message('Kore submodule not properly initialized, running get_dlc...')

        if build_machine.system() == 'windows'
            dlc_script = files('Kore/get_dlc.bat')
        else
            dlc_script = files('Kore/get_dlc')
        endif

        dlc_result = run_command([dlc_script], check: false)

        if dlc_result.returncode() != 0
            error('Failed to initialize Kore submodule. get_dlc failed with return code: ' + dlc_result.returncode().to_string())
        endif

        message('Kore submodule initialized successfully')
    else
        message('Kore submodule already initialized')
    endif

    kore_g = 'vulkan'

    if host_machine.system() == 'windows'
        kore_g = 'dx12'
    elif host_machine.system() == 'darwin'
        kore_g = 'metal'
    elif host_machine.system() == 'linux'
        kore_g = 'vulkan'
    else
        error('Unsupported system: ' + host_machine.system())
    endif

    output_meson_file = meson.current_source_dir() / 'output' / 'meson.build'

    if not fs.exists(output_meson_file)
        message('Generating Kore build files...')
        compile_kmake_result = run_command(
            compile_kmake_command + ['-g ' + kore_g],
        check: false)

        if compile_kmake_result.returncode() != 0
            error('Failed to compile Kore makefiles. Error: ' + compile_kmake_result.stderr())
        endif

        if not fs.exists(output_meson_file)
            error('kmake completed but did not generate output/meson.build')
        endif
    else
        message('Using existing Kore build files')
    endif

    spoopy_includes = [spoopy_includes, include_directories('include', 'include/kore2')]

    subdir('output')

    summary({
        'Renderer': 'Kore',
        'Target System': host_machine.system(),
        'Target CPU': host_machine.cpu_family(),
        'Graphics API': kore_g,
    }, section: 'Spoopy Renderer Build Summary')
else
    message('Kore renderer disabled')
endif
