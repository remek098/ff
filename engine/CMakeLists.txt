file(GLOB_RECURSE ENGINE_SOURCES 
    CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
)

set(IMGUI_DIR "${CMAKE_SOURCE_DIR}/vendor/imgui")

list(APPEND ENGINE_SOURCES
    "${IMGUI_DIR}/imgui.cpp"
    "${IMGUI_DIR}/imgui_demo.cpp"
    "${IMGUI_DIR}/imgui_draw.cpp"
    "${IMGUI_DIR}/imgui_tables.cpp"
    "${IMGUI_DIR}/imgui_widgets.cpp"
    "${IMGUI_DIR}/backends/imgui_impl_sdl3.cpp"
    "${IMGUI_DIR}/backends/imgui_impl_vulkan.cpp"
)

add_subdirectory("${CMAKE_SOURCE_DIR}/vendor/spdlog" "${CMAKE_BINARY_DIR}/vendor/spdlog" EXCLUDE_FROM_ALL)

add_library(engine STATIC ${ENGINE_SOURCES})

target_include_directories(engine PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}           
    ${Vulkan_INCLUDE_DIRS}                      # Vulkan SDK
    "${CMAKE_SOURCE_DIR}/vendor/vma/include"    # Vulkan Memory Allocator
    "${CMAKE_SOURCE_DIR}/vendor/sdl3/include"   # SDL3
    "${CMAKE_SOURCE_DIR}/vendor/vkb/src"        # VkBootstrap
    "${IMGUI_DIR}"                              # ImGui
    "${IMGUI_DIR}/backends"                     # ImGui backends
)


target_link_libraries(engine PUBLIC
    Vulkan::Vulkan
    SDL3::SDL3
    spdlog::spdlog
)