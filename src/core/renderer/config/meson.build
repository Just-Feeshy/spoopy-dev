renderer_backend_opts = {
	'metal' : get_option('renderer_metal'),
	'wgpu' : get_option('renderer_wgpu'),
}

renderer_backend_supported = {
	'metal' : host_machine.system() == 'darwin',
	'wgpu' : true,
}

renderer_api_backends = ['metal', 'wgpu']
renderer_api_entries = []
renderer_api_index = 0

foreach renderer_backend : renderer_api_backends
	renderer_api_entries += 'SPOOPY_RENDERER_API_@0@ = 1u << @1@,'.format(renderer_backend.to_upper(), renderer_api_index)
	renderer_api_index += 1
endforeach

default_renderer_backend = get_option('renderer_default')

foreach renderer_backend, supported : renderer_backend_supported
	if renderer_backend_opts[renderer_backend].enabled() and not supported
		error('@0@ renderer is not supported on @1@'.format(renderer_backend, host_machine.system()))
	endif
endforeach

renderer_backends_prio = host_machine.system() == 'darwin' ? ['metal', 'wgpu'] : ['wgpu']
enabled_renderer_backends = []

foreach renderer_backend : renderer_backends_prio
	if renderer_backend_opts[renderer_backend].allowed() and renderer_backend_supported[renderer_backend]
		enabled_renderer_backends += renderer_backend
	endif
endforeach

if default_renderer_backend == 'auto'
	if enabled_renderer_backends.length() == 0
		renderer_backends_prio_opts = []
		foreach renderer_backend : renderer_backends_prio
			renderer_backends_prio_opts += f'renderer_@renderer_backend@'
		endforeach

		renderer_backends_prio_opts = ', '.join(renderer_backends_prio_opts)
		error('Unable to pick out a default renderer backend. ' +
			f'Please make sure to have at least one of @renderer_backends_prio_opts@ enabled.')
	endif

	default_renderer_backend = enabled_renderer_backends[0]
endif

if not renderer_backend_supported[default_renderer_backend]
	error('@0@ renderer is not supported on @1@'.format(default_renderer_backend, host_machine.system()))
endif

assert(renderer_backend_opts[default_renderer_backend].allowed(),
	f'The default renderer backend @default_renderer_backend@ is not enabled')

ordered_renderer_backends = [default_renderer_backend]
foreach renderer_backend : enabled_renderer_backends
	if renderer_backend != default_renderer_backend
		ordered_renderer_backends += renderer_backend
	endif
endforeach

renderer_macro = []
renderer_available_api_entries = []
renderer_available_list_entries = []
renderer_compile_args = []
foreach renderer_backend : ordered_renderer_backends
	renderer_api = 'SPOOPY_RENDERER_API_@0@'.format(renderer_backend.to_upper())
	renderer_macro += 'RENDERER(@0@)'.format(renderer_backend.to_upper())
	renderer_available_api_entries += renderer_api
	renderer_available_list_entries += '@0@,'.format(renderer_api)
	renderer_compile_args += '-DSPOOPY_RENDERER_@0@'.format(renderer_backend.to_upper())
endforeach

renderer_api_macro = ' '.join(renderer_api_entries)
renderer_macro = ' '.join(renderer_macro)
renderer_available_api_macro = '(@0@)'.format(' | '.join(renderer_available_api_entries))
renderer_available_list_macro = ' '.join(renderer_available_list_entries)
renderer_default_api_macro = 'SPOOPY_RENDERER_API_@0@'.format(default_renderer_backend.to_upper())

config.set('SPOOPY_RENDERER_API_ENUMS', renderer_api_macro)
config.set('SPOOPY_RENDERER_API_AVAILABLE_MASK', renderer_available_api_macro)
config.set('SPOOPY_RENDERER_API_AVAILABLE_LIST', renderer_available_list_macro)
config.set('SPOOPY_RENDERER_API_DEFAULT', renderer_default_api_macro)
config.set('SPOOPY_HAS_RENDERERS', renderer_macro)
config.set_quoted('SPOOPY_HAS_RENDERER_DEFAULT', default_renderer_backend)

renderer_config = configuration_data()
renderer_config.set('SPOOPY_RENDERER_API_ENUMS', renderer_api_macro)
renderer_config.set('SPOOPY_RENDERER_API_AVAILABLE_MASK', renderer_available_api_macro)
renderer_config.set('SPOOPY_RENDERER_API_AVAILABLE_LIST', renderer_available_list_macro)
renderer_config.set('SPOOPY_RENDERER_API_DEFAULT', renderer_default_api_macro)
renderer_config.set('SPOOPY_HAS_RENDERERS', renderer_macro)
renderer_config.set_quoted('SPOOPY_HAS_RENDERER_DEFAULT', default_renderer_backend)

renderer_arg_languages = ['c']
if host_machine.system() == 'darwin'
	renderer_arg_languages += 'objc'
endif
add_project_arguments(renderer_compile_args, language: renderer_arg_languages)

sokol_backend_defines = {
	'metal' : 'SOKOL_METAL',
	'wgpu' : 'SOKOL_WGPU',
}

if not sokol_backend_defines.has_key(default_renderer_backend)
	error('No Sokol backend define is known for renderer backend: @0@'.format(default_renderer_backend))
endif

sokol_compile_args = ['-D@0@'.format(sokol_backend_defines[default_renderer_backend])]
