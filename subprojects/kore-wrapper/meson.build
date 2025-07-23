project('SpoopyKoreUnity', 'c', 'cpp', version: '1.0.0')

# Add Objective-C support only if we're on Darwin
if host_machine.system() == 'darwin'
    add_languages('objc', required: false)
endif

# Build Kore directly (since the source is in kore/ subdirectory)
kore_sources = files(
    'kore/sources/2d/2dunit.c',
    'kore/sources/audio/audio.c',
    'kore/sources/gpu/gpuunit.c', 
    'kore/sources/input/inputunit.c',
    'kore/sources/io/iounit.c',
    'kore/sources/math/mathunit.c',
    'kore/sources/mixer/mixerunit.c',
    'kore/sources/network/networkunit.c',
    'kore/sources/root/rootunit.c',
    'kore/sources/util/utilunit.c',
    'kore/sources/libs/offalloc/offalloc.c'
)

kore_includes = [
    include_directories('kore/includes')
]

kore_args = ['-DKORE_LZ4X']
kore_deps = []

# Platform-specific configuration for Darwin
if host_machine.system() == 'darwin'
    kore_sources += files(
        'kore/backends/system/apple/sources/appleunit.m',
        'kore/backends/system/macos/sources/macosunit.m',
        'kore/backends/system/macos/sources/audio.c',
        'kore/backends/system/posix/sources/posixunit.c',
        'kore/backends/gpu/metal/sources/metalunit.m'
    )
    
    kore_includes += [
        include_directories('kore/backends/system/apple/includes'),
        include_directories('kore/backends/system/macos/includes'),
        include_directories('kore/backends/system/macos/includes/kore3/backend'),
        include_directories('kore/backends/system/posix/includes'),
        include_directories('kore/backends/gpu/metal/includes')
    ]
    
    kore_args += ['-DKORE_METAL']
    
    # Apple framework dependencies
    kore_deps += [
        dependency('Metal'),
        dependency('MetalKit'),
        dependency('IOKit'),
        dependency('Cocoa'),
        dependency('AppKit'),
        dependency('CoreAudio'),
        dependency('Foundation')
    ]

# Platform-specific configuration for Emscripten
elif host_machine.system() == 'emscripten'
    kore_sources += files(
        'kore/backends/system/emscripten/sources/emscriptenunit.c',
        'kore/backends/gpu/webgpu/sources/webgpuunit.c'
    )
    
    kore_includes += [
        include_directories('kore/backends/system/emscripten/includes'),
        include_directories('kore/backends/gpu/webgpu/includes')
    ]
    
    kore_args += ['-DKORE_EMSCRIPTEN', '-DKORE_WEBGPU']
endif

# Build the base Kore library
kore_lib = static_library('kore',
    kore_sources,
    include_directories: kore_includes,
    dependencies: kore_deps,
    c_args: kore_args,
    cpp_args: kore_args,
    objc_args: kore_args,
    install: false
)

# Unity wrapper sources - bridge/glue code between Spoopy and Kore
wrapper_sources = files('spoopy_kore_bridge.c')

# Create unified library that combines Kore + Spoopy bridge code
unified_kore_lib = static_library('spoopy_kore_unity',
    sources: wrapper_sources,
    link_with: kore_lib,
    include_directories: [
        kore_includes,
        include_directories('../../include')  # Access to spoopy headers
    ],
    dependencies: kore_deps,
    c_args: kore_args,
    install: false
)

# Export unified dependency for the rest of Spoopy
kore_dep = declare_dependency(
    link_with: unified_kore_lib,
    include_directories: kore_includes,
    dependencies: kore_deps
)