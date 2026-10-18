file(GLOB_RECURSE TESTS_SOURCES 
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
)

# Tworzymy plik wykonywalny o nazwie 'main'
add_executable(main ${TESTS_SOURCES})

# Linkujemy nasz własny silnik 'engine'. 
# Dzięki temu CMake automatycznie podepnie też Vulkan i SDL3, które są przypisane do silnika!
target_link_libraries(main PRIVATE 
    engine
)

# AUTOMATYZACJA: Kopiowanie pliku SDL3.dll bezpośrednio obok main.exe w folderze /build/bin/
# add_custom_command(TARGET main POST_BUILD
#     COMMAND ${CMAKE_COMMAND} -E copy_if_different
#     "${VENDOR_DIR}/sdl3/lib/SDL3.dll"
#     $<TARGET_FILE_DIR:main>
#     COMMENT "Kopiowanie zewnętrznego SDL3.dll do folderu binarnego..."
# )

add_custom_command(TARGET main POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${VENDOR_DIR}/sdl3/lib/SDL3.dll"
    $<TARGET_FILE_DIR:main>
    COMMENT "Kopiowanie zewnetrznego SDL3.dll do folderu binarnego..."
)