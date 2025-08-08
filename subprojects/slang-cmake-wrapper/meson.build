project('slang-cmake-wrapper')

cmake = import('cmake')
slang_options = cmake.subproject_options()
slang_options.set_install(false)

if get_option('debug')
    slang_buildtype = 'Debug'
else
    slang_buildtype = 'Release'
endif

if host_machine.system() == 'darwin'
    slang_options.add_cmake_defines({'CMAKE_OSX_ARCHITECTURES': 'arm64'})
else
    slang_options.add_cmake_defines({'CMAKE_OSX_ARCHITECTURES': host_machine.cpu_family()})
endif

slang_options.add_cmake_defines({
    'CMAKE_BUILD_TYPE': slang_buildtype,
    'SLANG_LIB_TYPE': 'STATIC',
    'SLANG_SLANG_LLVM_FLAVOR': 'DISABLE',
    'SLANG_ENABLE_TESTS': 'OFF',
    'SLANG_ENABLE_EXAMPLES': 'OFF',
    'SLANG_ENABLE_GFX': 'OFF',
    'SLANG_ENABLE_REPLAYER': 'OFF',
    'SLANG_ENABLE_SLANG_RHI': 'OFF',
    'SLANG_ENABLE_SLANGC': 'ON',
})

# Disable graphics API backends to avoid cycles
slang_options.add_cmake_defines({
    'SLANG_ENABLE_SPIRV': 'OFF',
    'SLANG_ENABLE_DX12': 'OFF',
    'SLANG_ENABLE_METAL': 'OFF',
    'SLANG_ENABLE_OPTIX': 'OFF',
    'SLANG_ENABLE_CUDA': 'OFF',
})

# System library usage toggles (removed for simplicity)
# Exclusion toggles (removed for simplicity)
# String/path options (removed for simplicity)
# Override path options (removed for simplicity)

# Option validation (simplified)
# No validation needed for just the three core graphics API toggles

slang_subproject = cmake.subproject('slang', options : slang_options)

message('CMake targets:\n - ' + '\n - '.join(slang_subproject.target_list()))

slang_fiddle_gen = slang_subproject.target('slang-fiddle')
slang_cap_gen = slang_subproject.target('slang-capability-generator')
slang_dep = slang_subproject.dependency('slang')
slang_cmake_build_dir = slang_subproject.build_root()
