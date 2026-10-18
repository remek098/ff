cmake_minimum_required(VERSION 3.20)
project(VulkanFrameworkCcpp LANGUAGES CXX)

# WYMUSZENIE GENERATORA MULTI-CONFIG DLA NINJA
set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "" FORCE)


# Wymuszenie standardu C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# AUTOMATYZACJA: Wszystkie pliki wykonywalne (.exe) oraz biblioteki (.dll / .lib)
# trafią do jednego, wspólnego folderu wewnątrz katalogu budowania.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Automatyczne wyszukiwanie Vulkan SDK w systemie
find_package(Vulkan REQUIRED)


set(VENDOR_DIR "${CMAKE_CURRENT_SOURCE_DIR}/vendor")

# Konfiguracja SDL3 przed włączeniem (wyłączamy zbędne rzeczy, by kompilowało się szybciej)
set(SDL_SHARED ON CACHE BOOL "Build SDL as a shared library" FORCE)
set(SDL_STATIC OFF CACHE BOOL "Build SDL as a static library" FORCE)
set(SDL_TEST OFF CACHE BOOL "Disable SDL test applications" FORCE)

set(SDL_AUDIO ON CACHE BOOL "Disable audio subsystem" FORCE)
set(SDL_VULKAN ON CACHE BOOL "Enable Vulkan support" FORCE)

# Wyłączamy zbędne podsystemy SDL, by nie przeprowadzał dla nich testów:
set(SDL_CAMERA OFF CACHE BOOL "Disable camera subsystem" FORCE)
set(SDL_RENDER OFF CACHE BOOL "Disable SDL 2D render API (we use Vulkan)" FORCE)
set(SDL_OPENGLES OFF CACHE BOOL "Disable OpenGL ES support" FORCE)
set(SDL_OPENGL OFF CACHE BOOL "Disable OpenGL support" FORCE)


# Nakazujemy CMake wejść do submodułu i zbudować SDL3 z kodu źródłowego
add_subdirectory("${VENDOR_DIR}/sdl3")

# Dopiero teraz dodajemy Twoje podprojekty
add_subdirectory(engine)
add_subdirectory(tests)

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    # 1. LTO / IPO (Link Time Optimization)
    # Wymusza optymalizację między różnymi plikami .cpp w trakcie linkowania.
    # Sprawia, że wywołania funkcji z biblioteki 'engine' wewnątrz projektu 'tests' będą niesamowicie szybkie.
    include(CheckIPOSupported)
    check_ipo_supported(RESULT lto_supported OUTPUT error)
    if(lto_supported)
        message(STATUS "LTO / IPO jest wspierane i zostało włączone!")
        set_property(TARGET engine PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
        set_property(TARGET main PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
    endif()

    # 2. Wymuszenie instrukcji procesora (AVX2 / SIMD) dla Clang/GCC
    # Domyślnie CMake kompiluje kod tak, aby ruszył na każdym (nawet starym) procesorze.
    # Ta flaga mówi: "optymalizuj pod procesor, na którym aktualnie kompiluję program".
    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
        target_compile_options(engine PRIVATE -march=native)
        target_compile_options(main PRIVATE -march=native)
    endif()
endif()