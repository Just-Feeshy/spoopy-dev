project('mimalloc-cmake-wrapper')

cmake = import('cmake')

mi_opts = cmake.subproject_options()
mi_opts.set_install(false)
mi_opts.add_cmake_defines({
	'MI_BUILD_SHARED': false,
	'MI_BUILD_STATIC': true,
	'MI_BUILD_TESTS': false,
	'MI_BUILD_OBJECT': false,
	'MI_OVERRIDE': false,
	'MI_INSTALL_TOPLEVEL': false,
})

passthrough_toggles = [
	'xmalloc',
]

foreach toggle : passthrough_toggles
	opt = get_option(toggle)

	if not opt.auto()
		mi_opts.add_cmake_defines({
			'MI_@0@'.format(toggle.to_upper()): opt.enabled()
		})
	endif
endforeach

if host_machine.system() == 'windows'
	mi_opts.add_cmake_defines({
		'MI_WIN_REDIRECT_INLINE': false,
	})
endif

if host_machine.system() == 'darwin'
	mi_opts.add_cmake_defines({ 'CMAKE_OSX_ARCHITECTURES': 'arm64' })
endif

mimalloc_proj = cmake.subproject('mimalloc', options : mi_opts)
mimalloc_dep = mimalloc_proj.dependency('mimalloc-static')
