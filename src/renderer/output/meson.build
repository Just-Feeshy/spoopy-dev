inc_dirs = include_directories(
	  '../../../include',
	  '../Kore/backends/system/macos/includes/kore3/backend',
	  '../Kore/includes',
	  '../Kore/backends/system/apple/includes',
	  '../Kore/backends/system/macos/includes',
	  '../Kore/backends/system/posix/includes',
	  '../Kore/backends/gpu/metal/includes'
)

c_args = [
	  '-DKORE_LZ4X',
	  '-DKORE_METAL',
	  '-DKORE_NO_MAIN',
	  '-DNDEBUG',
]

cpp_args = [
	  '-DKORE_LZ4X',
	  '-DKORE_METAL',
	  '-DKORE_NO_MAIN',
	  '-DNDEBUG',
]

sources = [
	  '../src/spoopy_video.c',
	  '../Kore/backends/gpu/metal/sources/metalunit.m',
	  '../Kore/backends/system/apple/sources/appleunit.m',
	  '../Kore/backends/system/macos/sources/macosunit.m',
	  '../Kore/backends/system/posix/sources/posixunit.c',
	  '../Kore/sources/2d/2dunit.c',
	  '../Kore/sources/audio/audio.c',
	  '../Kore/sources/framebuffer/framebuffer.c',
	  '../Kore/sources/gpu/gpuunit.c',
	  '../Kore/sources/input/inputunit.c',
	  '../Kore/sources/io/iounit.c',
	  '../Kore/sources/libs/offalloc/offalloc.c',
	  '../Kore/sources/math/mathunit.c',
	  '../Kore/sources/mixer/mixerunit.c',
	  '../Kore/sources/network/networkunit.c',
	  '../Kore/sources/root/rootunit.c',
	  '../Kore/sources/util/utilunit.c',
]

target_files = sources

add_languages('objc', native: false)

objc_args = [
	  '-fobjc-arc',
	  '-DKORE_LZ4X',
	  '-DKORE_METAL',
	  '-DKORE_NO_MAIN',
	  '-DNDEBUG',
]

dependencies = [
	  dependency('Metal'),
	  dependency('QuartzCore'),
	  dependency('IOKit'),
	  dependency('Cocoa'),
	  dependency('AppKit'),
	  dependency('CoreAudio'),
	  dependency('CoreData'),
	  dependency('CoreMedia'),
	  dependency('CoreVideo'),
	  dependency('AVFoundation'),
	  dependency('Foundation'),
]

spoopy_renderer = library('spoopy_renderer',
	  target_files,
	  include_directories : inc_dirs,
	  c_args : c_args,
	  cpp_args : cpp_args,
	  objc_args : objc_args,
	  dependencies : dependencies,
	  install : false
)
