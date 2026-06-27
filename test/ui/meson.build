add_languages('cpp', native: false)

fs = import('fs')
cpp = meson.get_compiler('cpp')
cimgui_dir = 'cimgui'
cimgui_deps = []
cimgui_objcpp_args = []

cimgui_srcs = [
	cimgui_dir / 'cimgui.cpp',
	cimgui_dir / 'imgui/imgui.cpp',
	cimgui_dir / 'imgui/imgui_draw.cpp',
	cimgui_dir / 'imgui/imgui_demo.cpp',
	cimgui_dir / 'imgui/imgui_widgets.cpp',
]

if host_machine.system() == 'darwin'
	add_languages('objcpp', native: false)
	cimgui_srcs += 'apple/ui_impl.mm'
	cimgui_deps += dependency('appleframeworks', modules: ['Foundation', 'Metal', 'QuartzCore'])
endif

if fs.exists(meson.current_source_dir() / cimgui_dir / 'imgui/imgui_tables.cpp')
	cimgui_srcs += cimgui_dir / 'imgui/imgui_tables.cpp'
endif

cimgui_inc = [
	include_directories(
		cimgui_dir,
		cimgui_dir / 'imgui',
		cimgui_dir / 'imgui/backends',
		'.',
	),
	spoopy_includes,
]

cimgui_common_args = [
	'-DIMGUI_USER_CONFIG="../cimconfig.h"',
	'-DIMGUI_DISABLE_OBSOLETE_FUNCTIONS=1',
]

cimgui_cpp_args = cimgui_common_args
cimgui_objcpp_args = cimgui_common_args

if host_machine.system() == 'darwin'
	cimgui_objcpp_args += macos_min_arg
	cimgui_objcpp_args += ['-target', 'arm64-apple-darwin']
	cimgui_objcpp_args += [
		'-fobjc-arc',
		'-Wno-nullability-completeness',
	]
endif

cimgui_cpp_args += cpp.get_supported_arguments('-Wno-unused-function')

if host_machine.system() == 'windows'
	cimgui_cpp_args += '-DIMGUI_IMPL_API=extern "C" __declspec(dllexport)'
else
	cimgui_cpp_args += '-DIMGUI_IMPL_API=extern "C"'
endif

cimgui_lib = library(
	'cimgui',
	cimgui_srcs,
	dependencies : cimgui_deps,
	include_directories : cimgui_inc,
	cpp_args : cimgui_cpp_args,
	objcpp_args : cimgui_objcpp_args,
	override_options : ['cpp_std=c++11'],
	name_prefix : '',
	install : false,
)

cimgui_dep = declare_dependency(
	link_with : cimgui_lib,
	dependencies : cimgui_deps,
	include_directories : cimgui_inc,
)
