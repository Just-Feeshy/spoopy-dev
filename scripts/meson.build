check_submodules_script = find_program('check_submodules.js')
check_submodules_command = [check_submodules_script]

compile_kmake_script = find_program('compile_kmake.js')
compile_kmake_command = [compile_kmake_script]

default_options_script = find_program('default_options.js')
default_options_command = [default_options_script, '--language', 'c', '--build-type', 'auto']
subproject_default_options = run_command(default_options_command, check: true).stdout().strip().split('\n')
