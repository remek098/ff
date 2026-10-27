file(GLOB_RECURSE TESTS_SOURCES 
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
)

add_executable(test ${TESTS_SOURCES})

target_link_libraries(test PRIVATE 
    engine
)
