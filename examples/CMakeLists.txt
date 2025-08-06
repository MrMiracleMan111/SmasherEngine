
# Generate Manifest Header for Basic Resource Load
find_package(Python3 REQUIRED COMPONENTS Interpreter)
set(RESOURCE_EXAMPLE_MANIFEST_JSON "${CMAKE_CURRENT_SOURCE_DIR}/Manifest.json")
set(RESOURCE_EXAMPLE_MANIFEST_HEADER "${CMAKE_CURRENT_SOURCE_DIR}/Manifest.h")

add_custom_command(
    OUTPUT Manifest.h
    COMMAND ${Python3_EXECUTABLE} 
        ${MANIFEST_GENERATOR}
        --files ${RESOURCE_EXAMPLE_MANIFEST_JSON}
        --out ${RESOURCE_EXAMPLE_MANIFEST_HEADER}
        --resources "${CMAKE_SOURCE_DIR}/examples"

    DEPENDS "${MANIFEST_GENERATOR}" "${RESOURCE_EXAMPLE_MANIFEST_JSON}"
    COMMENT "Generating Manifest header file..."
    VERBATIM
)

# Basic Window Example
add_executable (BasicWindowExample "basic_window/main.cpp" "basic_window/src/ExampleWindowGameState.cpp" )
foreach(CONFIG ${CMAKE_CONFIGURATION_TYPES})
    string(TOUPPER ${CMAKE_BUILD_TYPE} CONFIG_UPPER)
    set_target_properties(BasicWindowExample PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY_${CONFIG_UPPER} "${SMASHER_ENGINE_OUTPUT_DIR}"
    )
endforeach()
set_target_properties(BasicWindowExample PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG "${SMASHER_ENGINE_OUTPUT_DIR}")
target_compile_features(BasicWindowExample PUBLIC cxx_std_20)
target_include_directories (BasicWindowExample PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}" "basic_window/include" "basic_window/src")
target_link_libraries( BasicWindowExample PRIVATE SmasherEngine )

# Basic Resource Load Example
add_custom_target(generate_manifest_BasicResourceLoadExample DEPENDS Manifest.h)
add_executable (BasicResourceLoadExample "basic_resource_load/main.cpp" "basic_resource_load/src/ExampleResourcesGameState.cpp" )

set_target_properties(BasicResourceLoadExample PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG "${SMASHER_ENGINE_OUTPUT_DIR}")
add_dependencies(BasicResourceLoadExample generate_manifest_BasicResourceLoadExample)
target_compile_features(BasicResourceLoadExample PUBLIC cxx_std_20)
target_include_directories (BasicResourceLoadExample PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}" "basic_resource_load/include" "basic_resource_load/src")
target_link_libraries( BasicResourceLoadExample PRIVATE SmasherEngine )
