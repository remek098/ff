file(GLOB_RECURSE TESTS_SOURCES 
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
)

add_executable(main ${TESTS_SOURCES})

target_link_libraries(main PRIVATE 
    engine
)


if(WIN32)
    add_custom_command(TARGET your_target POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different 
        "${CMAKE_SOURCE_DIR}/vendor/sdl3/lib/SDL3.dll" 
        $<TARGET_FILE_DIR:your_target>
    )
endif()