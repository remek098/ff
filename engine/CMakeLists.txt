file(GLOB_RECURSE ENGINE_SOURCES 
    CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
)

set(IMGUI_DIR "${CMAKE_SOURCE_DIR}/vendor/imgui")

# list(APPEND ENGINE_SOURCES
#     "${CMAKE_SOURCE_DIR}/vendor/vkb/src/VkBootstrap.cpp"
#     "${IMGUI_DIR}/imgui.cpp"
#     "${IMGUI_DIR}/imgui_demo.cpp"
#     "${IMGUI_DIR}/imgui_draw.cpp"
#     "${IMGUI_DIR}/imgui_tables.cpp"
#     "${IMGUI_DIR}/imgui_widgets.cpp"
#     "${IMGUI_DIR}/backends/imgui_impl_sdl3.cpp"
#     "${IMGUI_DIR}/backends/imgui_impl_vulkan.cpp"
# )
set(VENDOR_SOURCES
    "${CMAKE_SOURCE_DIR}/vendor/vkb/src/VkBootstrap.cpp"
    "${IMGUI_DIR}/imgui.cpp"
    "${IMGUI_DIR}/imgui_demo.cpp"
    "${IMGUI_DIR}/imgui_draw.cpp"
    "${IMGUI_DIR}/imgui_tables.cpp"
    "${IMGUI_DIR}/imgui_widgets.cpp"
    "${IMGUI_DIR}/backends/imgui_impl_sdl3.cpp"
    "${IMGUI_DIR}/backends/imgui_impl_vulkan.cpp"
)

add_library(vendor_libs STATIC ${VENDOR_SOURCES})
set_target_properties(vendor_libs PROPERTIES UNITY_BUILD ON)

target_include_directories(vendor_libs PUBLIC
    ${Vulkan_INCLUDE_DIRS}
    "${CMAKE_SOURCE_DIR}/vendor/sdl3/include"
    "${CMAKE_SOURCE_DIR}/vendor/vkb/src"
    "${IMGUI_DIR}"
    "${IMGUI_DIR}/backends"
)
target_link_libraries(vendor_libs PUBLIC 
    Vulkan::Vulkan 
    SDL3::SDL3
)


set(SPDLOG_BUILD_SHARED OFF CACHE BOOL "" FORCE)
set(SPDLOG_BUILD_EXAMPLE OFF CACHE BOOL "" FORCE)
set(SPDLOG_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(SPDLOG_BUILD_BENCH OFF CACHE BOOL "" FORCE)

add_subdirectory("${CMAKE_SOURCE_DIR}/vendor/spdlog" "${CMAKE_BINARY_DIR}/vendor/spdlog" EXCLUDE_FROM_ALL)
add_subdirectory("${CMAKE_SOURCE_DIR}/vendor/glm" "${CMAKE_BINARY_DIR}/vendor/glm" EXCLUDE_FROM_ALL)

# add_library(engine STATIC ${ENGINE_SOURCES})

# target_include_directories(engine PUBLIC 
#     ${CMAKE_CURRENT_SOURCE_DIR}
#     ${CMAKE_CURRENT_SOURCE_DIR}/core           
#     ${Vulkan_INCLUDE_DIRS}                      # Vulkan SDK
#     "${CMAKE_SOURCE_DIR}/vendor/vma/include"    # Vulkan Memory Allocator
#     "${CMAKE_SOURCE_DIR}/vendor/sdl3/include"   # SDL3
#     "${CMAKE_SOURCE_DIR}/vendor/vkb/src"        # VkBootstrap
#     "${IMGUI_DIR}"                              # ImGui
#     "${IMGUI_DIR}/backends"                     # ImGui backends
# )

# target_link_libraries(engine PUBLIC
#     Vulkan::Vulkan
#     SDL3::SDL3
#     spdlog::spdlog
# )

add_library(engine STATIC ${ENGINE_SOURCES})

target_compile_definitions(engine PRIVATE
    GLM_FORCE_DEPTH_ZERO_TO_ONE
)

target_include_directories(engine PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/core           
    "${CMAKE_SOURCE_DIR}/vendor/vma/include"
)


target_link_libraries(engine PUBLIC
    vendor_libs
    spdlog::spdlog
    glm::glm
)

# target_precompile_headers(engine PRIVATE
#     <vector>
#     <deque>
#     <string>
#     <memory>
#     <vulkan/vulkan.h>
#     <SDL3/SDL.h>
#     <spdlog/spdlog.h>
#     <glm/glm.hpp>
#     <glm/gtc/matrix_transform.hpp>
#     <glm/gtc/quaternion.hpp>
#     <glm/gtc/round.hpp>
#     <glm/gtc/type_ptr.hpp>
# )

target_precompile_headers(engine PRIVATE pch.h)