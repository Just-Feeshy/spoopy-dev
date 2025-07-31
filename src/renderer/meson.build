opt_kore = get_option('enable_kore')

if opt_kore.enabled()
    # Check submodules first
    sm_check = run_command(check_submodules_command, check: false)

    if sm_check.returncode() != 0
        warning('Submodule check command failed')
    elif sm_check.stdout() != ''
        foreach line : sm_check.stdout().strip().split('\n')
            warning('Submodule: ' + line)
        endforeach

        build_os = build_machine.system()
        kore_dir = meson.current_source_root() / 'kore'

        if build_os == 'windows'
            dlc_script = files('kore/get_dlc.bat')
        else
            dlc_script = files('kore/get_dlc')
        endif

        if fs.is_file(dlc_script)
            message('Acquiring Kore DLC content...')
            dlc_result = run_command([dlc_script],
                                   cwd: kore_dir,
                                   check: false)

            if dlc_result.returncode() == 0
                message('Kore DLC content acquired successfully')
            else
                warning('Failed to acquire Kore DLC: ' + dlc_result.stderr())
            endif
        else
            warning('Kore DLC script not found: ' + dlc_script)
        endif
    else
        message('All submodules are properly initialized')
    endif


    if build_machine.system() == 'windows'
        kmake_script = files('Kore/kmake.bat')
    else
        kmake_script = files('Kore/kmake.sh')
    endif

    if host_machine.system() == 'emscripten'
        kore_g = '-g webgpu'
    elif host_machine.system() == 'windows'
        kore_g = '-g d3d12'
    elif host_machine.system() == 'darwin'
        kore_g = '-g metal'
    else
        kore_g = '-g vulkan'
    endif

    output_dir = 'output'

    compile_kmake = run_command([kmake_script,
        kore_g,
        '--lib',
        '--json',
        '--to ' + output_dir,
    ], check: false)

    if compile_kmake.returncode() == 0
        target_system = target_machine.system()
        target_cpu = target_machine.cpu_family()

        message('Kore build completed successfully')
        output_json = files(output_dir / 'spoopy-renderer.json')
        output_config = json_reader(output_json)

        output_includes = output_config.get('includes')
        output_deps = output_config.get('libraries')
        output_defines = output_config.get('defines')
        output_files = output_config.get('files')
        output_c_flags = output_config.get('cFlags')
        output_kong_dirs = output_config.get('kongDirs')

        k_compile_args = []
        k_link_args = []
        k_deps = []
        k_sources = []
        k_includes = []

        foreach inc : output_includes
            k_includes += include_directories(inc)
        endforeach

        foreach define : output_defines
            k_compile_args += '-D' + define
        endforeach

        foreach flag : output_c_flags
            k_compile_args += flag
        endforeach

        if target_system == 'darwin'
            foreach lib : libraries
                k_deps += dependency('appleframeworks', modules: lib)
            endforeach
        endif

        foreach file : files
            include_file = true

            if target_system != 'darwin'
                if file.contains('/apple/') or file.contains('/macos/') or file.contains('/ios/')
                    include_file = false
                elif file.contains('/metal/')
                    include_file = false
                endif
            endif

            if file.contains('/posix/')
                if target_system == 'windows'
                    include_file = false
                endif
            endif

            if include_file
                k_sources += file
            endif
        endforeach
        if target_system == 'darwin'
            k_compile_args += '-DKORE_MACOS'
            k_compile_args += '-DKORE_POSIX'
            if target_cpu == 'aarch64'
                k_compile_args += '-DKORE_ARM64'
            endif
        endif

        spoopy_renderer_lib = static_library('spoopy-renderer',
            sources: k_sources,
            include_directories: k_includes,
            dependencies: k_deps,
            c_args: k_compile_args,
            link_with: k_link_args, # Do something with this maybe? Idk.
            install: false
        )

        summary({
            'Project': 'Spoopy Renderer',
            'Target System': target_system,
            'Target CPU': target_cpu,
            'Source Files': k_sources.length(),
            'Dependencies': k_deps.length(),
            'Includes': k_includes.length(),
            'Compile Args': k_compile_args.length(),
            'Link Args': k_link_args.length(),
            'Compiler Defines': output_defines.length(),
        }, section: 'Spoopy Renderer Build Summary')
    else
        error('❌ kmake failed with return code: ' + compile_kmake.returncode().to_string())
    endif
endif
