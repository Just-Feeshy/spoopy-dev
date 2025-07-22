cmake_minimum_required(VERSION 3.16)
project(Kore C CXX)

set(KORE_ARGS "")
set(KORE_INCLUDES "")
set(KORE_SOURCES "")
set(KORE_DEPS "")
set(KORE_FLAGS "")
set(KORE_LINK_ARGS "")

function(add_kore_define name result_var)
    set(${result_var} "-DKORE_${name}" PARENT_SCOPE)
endfunction()

list(APPEND KORE_INCLUDES "${CMAKE_CURRENT_SOURCE_DIR}/kore/includes")
file(GLOB_RECURSE KORE_HEADER_FILES "${CMAKE_CURRENT_SOURCE_DIR}/kore/includes/**/*.h")
list(APPEND KORE_SOURCES ${KORE_HEADER_FILES})

function(add_unit name)
    file(GLOB_RECURSE UNIT_DEPS "${CMAKE_CURRENT_SOURCE_DIR}/kore/sources/${name}/**")
    file(GLOB UNIT_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/kore/sources/${name}/*unit.c*")

    set(UNIT_SOURCES_OUT ${UNIT_SOURCES} PARENT_SCOPE)
    set(UNIT_DEPS_OUT ${UNIT_DEPS} PARENT_SCOPE)
endfunction()

set(UNIT_NAMES "2d" "gpu" "input" "io" "math" "mixer" "network" "root" "util")
foreach(name IN LISTS UNIT_NAMES)
    add_unit(${name})
    list(APPEND KORE_SOURCES ${UNIT_SOURCES_OUT})
    list(APPEND KORE_DEPS ${UNIT_DEPS_OUT})
endforeach()

add_kore_define("LZ4X" LZ4X_DEFINE)
list(APPEND KORE_ARGS ${LZ4X_DEFINE})
list(APPEND KORE_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/kore/sources/libs/offalloc/offalloc.c")

function(add_backend name)
    set(backend_path "${CMAKE_CURRENT_SOURCE_DIR}/kore/backends/${name}")

    list(APPEND KORE_INCLUDES "${backend_path}/includes")
    file(GLOB_RECURSE INCLUDE_DEPS "${backend_path}/includes/**")
    file(GLOB SOURCE_DEPS "${backend_path}/sources/*")
    file(GLOB C_SOURCES "${backend_path}/sources/*unit.c*")
    file(GLOB WINRT_SOURCES "${backend_path}/sources/*unit.winrt.c*")
    file(GLOB OBJC_SOURCES "${backend_path}/sources/*unit.m")

    set(BACKEND_SOURCES ${C_SOURCES} ${WINRT_SOURCES} ${OBJC_SOURCES})
    set(BACKEND_DEPS ${INCLUDE_DEPS} ${SOURCE_DEPS})

    set(BACKEND_INCLUDES_OUT ${KORE_INCLUDES} PARENT_SCOPE)
    set(BACKEND_SOURCES_OUT ${BACKEND_SOURCES} PARENT_SCOPE)
    set(BACKEND_DEPS_OUT ${BACKEND_DEPS} PARENT_SCOPE)
endfunction()

function(add_simple_backend name)
    file(GLOB BACKEND_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/kore/backends/${name}/sources/*")
    list(APPEND KORE_SOURCES ${BACKEND_SOURCES})
    set(KORE_SOURCES ${KORE_SOURCES} PARENT_SCOPE)
endfunction()

function(add_backend_with_flags name)
    set(options "")
    set(oneValueArgs "")
    set(multiValueArgs C_FLAGS CPP_FLAGS LINK_FLAGS)
    cmake_parse_arguments(BACKEND "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    set(backend_path "${CMAKE_CURRENT_SOURCE_DIR}/kore/backends/${name}")
    list(APPEND KORE_INCLUDES "${backend_path}/includes")
    file(GLOB BACKEND_SOURCES "${backend_path}/sources/*unit.*")

    set(BACKEND_INCLUDES_OUT ${KORE_INCLUDES} PARENT_SCOPE)
    set(BACKEND_SOURCES_OUT ${BACKEND_SOURCES} PARENT_SCOPE)
    set(BACKEND_C_ARGS_OUT ${BACKEND_C_FLAGS} PARENT_SCOPE)
    set(BACKEND_CPP_ARGS_OUT ${BACKEND_CPP_FLAGS} PARENT_SCOPE)
    set(BACKEND_LINK_ARGS_OUT ${BACKEND_LINK_FLAGS} PARENT_SCOPE)
endfunction()

if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    add_backend("system/apple")
    add_backend("system/macos")
    add_backend("system/posix")

    add_backend("gpu/metal")
    add_kore_define("METAL" METAL_DEFINE)
    list(APPEND KORE_ARGS ${METAL_DEFINE})

    find_library(METAL_FRAMEWORK Metal)
    find_library(METALKIT_FRAMEWORK MetalKit)
    find_library(IOKIT_FRAMEWORK IOKit)
    find_library(COCOA_FRAMEWORK Cocoa)
    find_library(APPKIT_FRAMEWORK AppKit)
    find_library(COREAUDIO_FRAMEWORK CoreAudio)
    find_library(COREDATA_FRAMEWORK CoreData)
    find_library(COREMEDIA_FRAMEWORK CoreMedia)
    find_library(COREVIDEO_FRAMEWORK CoreVideo)
    find_library(AVFOUNDATION_FRAMEWORK AVFoundation)
    find_library(FOUNDATION_FRAMEWORK Foundation)

    list(APPEND KORE_DEPS
        ${METAL_FRAMEWORK}
        ${METALKIT_FRAMEWORK}
        ${IOKIT_FRAMEWORK}
        ${COCOA_FRAMEWORK}
        ${APPKIT_FRAMEWORK}
        ${COREAUDIO_FRAMEWORK}
        ${COREDATA_FRAMEWORK}
        ${COREMEDIA_FRAMEWORK}
        ${COREVIDEO_FRAMEWORK}
        ${AVFOUNDATION_FRAMEWORK}
        ${FOUNDATION_FRAMEWORK}
    )

elseif(CMAKE_SYSTEM_NAME STREQUAL "Emscripten")
    add_kore_define("EMSCRIPTEN" EMSCRIPTEN_DEFINE)
    list(APPEND KORE_ARGS ${EMSCRIPTEN_DEFINE})

    add_backend("system/emscripten")
    list(APPEND KORE_LINK_ARGS "-sUSE_GLFW=2")

    add_backend("gpu/webgpu")
    add_kore_define("WEBGPU" WEBGPU_DEFINE)
    list(APPEND KORE_ARGS ${WEBGPU_DEFINE})

    set(WEBGPU_FLAGS "--use-port=emdawnwebgpu")
    add_backend_with_flags("webgpu"
        C_FLAGS ${WEBGPU_FLAGS}
        CPP_FLAGS ${WEBGPU_FLAGS}
        LINK_FLAGS ${WEBGPU_FLAGS}
    )

    list(APPEND KORE_LINK_ARGS "-sASYNCIFY" "-sEXIT_RUNTIME")
endif()

add_library(kore STATIC ${KORE_SOURCES})

target_include_directories(kore PUBLIC ${KORE_INCLUDES})
target_compile_definitions(kore PRIVATE ${KORE_ARGS})

if(DEFINED BACKEND_C_ARGS_OUT)
    target_compile_options(kore PRIVATE $<$<COMPILE_LANGUAGE:C>:${BACKEND_C_ARGS_OUT}>)
endif()

if(DEFINED BACKEND_CPP_ARGS_OUT)
    target_compile_options(kore PRIVATE $<$<COMPILE_LANGUAGE:CXX>:${BACKEND_CPP_ARGS_OUT}>)
endif()

if(KORE_DEPS)
    target_link_libraries(kore PUBLIC ${KORE_DEPS})
endif()

if(KORE_LINK_ARGS OR BACKEND_LINK_ARGS_OUT)
    target_link_options(kore PRIVATE ${KORE_LINK_ARGS} ${BACKEND_LINK_ARGS_OUT})
endif()

set_target_properties(kore PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION 0
    CXX_VISIBILITY_PRESET default
    C_VISIBILITY_PRESET default
)

add_library(Kore::kore ALIAS kore)
