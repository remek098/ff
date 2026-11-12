file(GLOB_RECURSE EDITOR_SOURCES 
    CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
)

add_executable(editor ${EDITOR_SOURCES})

# Linking against engine automatically grants access to:
# - Engine APIs
# - vendor_libs (ImGui, SDL3, VkBootstrap, Vulkan) via public propagation
# - spdlog and glm
# - BUILD_WITH_EDITOR compile definition (if engine defines it PUBLIC)
target_link_libraries(editor PRIVATE 
    engine
)

# Optional: ensure the editor target always has editor support enabled,
# even if someone globally toggled BUILD_WITH_EDITOR to OFF for a shipping build.
if(NOT BUILD_WITH_EDITOR)
    target_compile_definitions(editor PRIVATE BUILD_WITH_EDITOR)
endif()