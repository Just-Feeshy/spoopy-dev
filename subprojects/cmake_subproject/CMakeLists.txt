cmake_minimum_required(VERSION 3.15)
project(visuals LANGUAGES C)

add_library(visuals STATIC
    shader.c
)

target_include_directories(visuals PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../include>
    $<INSTALL_INTERFACE:include>
)

target_compile_features(visuals PUBLIC c_std_99)
include(GNUInstallDirs)

install(TARGETS visuals
    EXPORT visuals-targets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)


install(EXPORT visuals-targets
    FILE visuals-targets.cmake
    NAMESPACE visuals::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/visuals
)
