cmake = import('cmake')

# Configure CMake subproject options
cmake_opts = cmake.subproject_options()
cmake_opts.add_cmake_defines({'CMAKE_POSITION_INDEPENDENT_CODE': 'ON'})

visuals_sub = cmake.subproject('cmake_subproject', options: cmake_opts)
visuals_dep = visuals_sub.dependency('visuals')
