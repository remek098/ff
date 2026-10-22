file(GLOB_RECURSE TESTS_SOURCES 
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
)

add_executable(main ${TESTS_SOURCES})

target_link_libraries(main PRIVATE 
    engine
)


if(WIN32)
    add_custom_command(TARGET main POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${VENDOR_DIR}/sdl3/lib/SDL3.dll"
        $<TARGET_FILE_DIR:main>
        COMMENT "Copying SDL3.dll to output directory..."
    )
endif()