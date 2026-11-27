# function(add_slang_shaders TARGET_NAME SHADER_SOURCE_DIR)
#     # Grab all .slang files (recursively)
#     file(GLOB_RECURSE SLANG_SOURCES CONFIGURE_DEPENDS "${SHADER_SOURCE_DIR}/*.slang")

#     # Output to <build_dir>/bin/shaders (matches your launch.json cwd)
#     set(SHADER_OUTPUT_DIR "${CMAKE_BINARY_DIR}/bin/shaders")
#     set(COMPILED_SHADERS "")

#     foreach(SHADER_PATH ${SLANG_SOURCES})
#         # Preserve relative subfolder structure (e.g., shaders/pbr/mesh.slang -> bin/shaders/pbr/mesh.spv)
#         file(RELATIVE_PATH REL_PATH "${SHADER_SOURCE_DIR}" "${SHADER_PATH}")
#         cmake_path(REPLACE_EXTENSION REL_PATH ".spv" OUTPUT_VARIABLE REL_SPV_PATH)
#         set(OUTPUT_SPV "${SHADER_OUTPUT_DIR}/${REL_SPV_PATH}")

#         # Ensure output subdirectory exists
#         cmake_path(GET OUTPUT_SPV PARENT_PATH OUTPUT_SUBDIR)

#         add_custom_command(
#             OUTPUT "${OUTPUT_SPV}"
#             COMMAND ${CMAKE_COMMAND} -E make_directory "${OUTPUT_SUBDIR}"
#             COMMAND "${SLANGC_EXECUTABLE}" "${SHADER_PATH}"
#                     -target spirv
#                     -profile spirv_1_6
#                     -emit-spirv-directly
#                     $<$<CONFIG:Debug>:-g2 -O0>
#                     $<$<NOT:$<CONFIG:Debug>>:-O3>
#                     -o "${OUTPUT_SPV}"
#             DEPENDS "${SHADER_PATH}"
#             COMMENT "Compiling Slang shader: ${REL_PATH} -> ${REL_SPV_PATH}"
#             VERBATIM
#         )

#         list(APPEND COMPILED_SHADERS "${OUTPUT_SPV}")
#     endforeach()

#     # Create a custom target and attach it to your engine/app target
#     add_custom_target(${TARGET_NAME}_shaders DEPENDS ${COMPILED_SHADERS})
#     add_dependencies(${TARGET_NAME} ${TARGET_NAME}_shaders)
# endfunction()

function(add_slang_shaders TARGET_NAME SHADER_SOURCE_DIR)
    file(GLOB_RECURSE SLANG_SOURCES CONFIGURE_DEPENDS "${SHADER_SOURCE_DIR}/*.slang")

    set(SHADER_OUTPUT_DIR "${CMAKE_BINARY_DIR}/bin/shaders")
    set(COMPILED_SHADERS "")

    foreach(SHADER_PATH ${SLANG_SOURCES})
        file(RELATIVE_PATH REL_PATH "${SHADER_SOURCE_DIR}" "${SHADER_PATH}")
        cmake_path(REPLACE_EXTENSION REL_PATH ".spv" OUTPUT_VARIABLE REL_SPV_PATH)
        set(OUTPUT_SPV "${SHADER_OUTPUT_DIR}/${REL_SPV_PATH}")

        cmake_path(GET OUTPUT_SPV PARENT_PATH OUTPUT_SUBDIR)

        add_custom_command(
            OUTPUT "${OUTPUT_SPV}"
            COMMAND ${CMAKE_COMMAND} -E make_directory "${OUTPUT_SUBDIR}"
            COMMAND "${SLANGC_EXECUTABLE}" "${SHADER_PATH}"
                    -target spirv
                    -profile spirv_1_6
                    -emit-spirv-directly
                    "$<$<CONFIG:Debug>:-g2;-O0>"
                    "$<$<NOT:$<CONFIG:Debug>>:-O3>"
                    -o "${OUTPUT_SPV}"
            DEPENDS "${SHADER_PATH}"
            COMMENT "Compiling Slang shader: ${REL_PATH} -> ${REL_SPV_PATH}"
            COMMAND_EXPAND_LISTS
            VERBATIM
        )

        list(APPEND COMPILED_SHADERS "${OUTPUT_SPV}")
    endforeach()

    add_custom_target(${TARGET_NAME}_shaders DEPENDS ${COMPILED_SHADERS})
    add_dependencies(${TARGET_NAME} ${TARGET_NAME}_shaders)
endfunction()