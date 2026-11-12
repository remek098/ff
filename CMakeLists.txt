cmake_minimum_required(VERSION 3.20)
project(FfEngineWorkspace LANGUAGES CXX)

option(BUILD_WITH_EDITOR "Build engine with editor support and script name reflection" ON)

set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "" FORCE)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# all files will land in the output directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

find_package(Vulkan REQUIRED)


set(VENDOR_DIR "${CMAKE_CURRENT_SOURCE_DIR}/vendor")

# config SDL3 to what we need and to speed up launch
set(SDL_SHARED ON CACHE BOOL "Build SDL as a shared library" FORCE)
set(SDL_STATIC OFF CACHE BOOL "Build SDL as a static library" FORCE)
set(SDL_TEST OFF CACHE BOOL "Disable SDL test applications" FORCE)

set(SDL_AUDIO ON CACHE BOOL "Disable audio subsystem" FORCE)
set(SDL_VULKAN ON CACHE BOOL "Enable Vulkan support" FORCE)

set(SDL_CAMERA OFF CACHE BOOL "Disable camera subsystem" FORCE)
set(SDL_RENDER OFF CACHE BOOL "Disable SDL 2D render API (we use Vulkan)" FORCE)
set(SDL_OPENGLES OFF CACHE BOOL "Disable OpenGL ES support" FORCE)
set(SDL_OPENGL OFF CACHE BOOL "Disable OpenGL support" FORCE)


set(SDL_LIBC ON CACHE BOOL "Use system C library" FORCE)
add_subdirectory("${VENDOR_DIR}/sdl3")


add_subdirectory(engine)
add_subdirectory(tests)
add_subdirectory(editor)

if(APPLE)
    execute_process(
        COMMAND brew --prefix
        OUTPUT_VARIABLE HOMEBREW_PREFIX
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )

    # Wstrzyknij RPATH bezpośrednio do końcowej binarki 'test'
    target_link_directories(test PRIVATE "${HOMEBREW_PREFIX}/lib")
    
    set_target_properties(test PROPERTIES
        BUILD_WITH_INSTALL_RPATH TRUE
        INSTALL_RPATH "${HOMEBREW_PREFIX}/lib"
        ENVIRONMENT "VK_LAYER_PATH=${HOMEBREW_PREFIX}/share/vulkan/explicit_layer.d"
    )
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    # 1. LTO / IPO (Link Time Optimization)
    include(CheckIPOSupported)
    check_ipo_supported(RESULT lto_supported OUTPUT error)
    if(lto_supported)
        message(STATUS "LTO / IPO jest wspierane i zostało włączone!")
        set_property(TARGET engine PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
        set_property(TARGET test PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
    endif()

    # AVX2 / SIMD for Clang/GCC
    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
        target_compile_options(engine PRIVATE -march=native)
        target_compile_options(test PRIVATE -march=native)
    endif()
endif()