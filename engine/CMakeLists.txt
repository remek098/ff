file(GLOB_RECURSE ENGINE_SOURCES 
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
)

# Tworzymy bibliotekę statyczną o nazwie 'engine'
add_library(engine STATIC ${ENGINE_SOURCES})

# Nagłówki, które silnik potrzebuje do skompilowania samego siebie
target_include_directories(engine PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}           
    ${Vulkan_INCLUDE_DIRS}                      # Vulkan SDK
    "${CMAKE_SOURCE_DIR}/vendor/vma/include"    # Vulkan Memory Allocator
    "${CMAKE_SOURCE_DIR}/vendor/sdl3/include"   # SDL3
    "${CMAKE_SOURCE_DIR}/vendor/vkb/src"        # VkBootstrap
)



# # Linkowanie bibliotek do silnika przy użyciu PEŁNYCH ŚCIEŻEK
# target_link_libraries(engine PUBLIC
#     Vulkan::Vulkan
#     "${VENDOR_DIR}/sdl3/lib/SDL3.lib"     # <--- KLUCZOWA ZMIANA: Pełna ścieżka zamiast samego "-lSDL3"
# )

target_link_libraries(engine PUBLIC
    Vulkan::Vulkan
    SDL3::SDL3
)