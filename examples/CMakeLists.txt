add_executable (BasicWindowExample "basic_window/main.cpp" "basic_window/src/ExampleWindowGameState.cpp" )
target_compile_features(BasicWindowExample PUBLIC cxx_std_20)
target_include_directories (BasicWindowExample PUBLIC "basic_window/include" "basic_window/src")
target_link_libraries( BasicWindowExample PRIVATE SmasherEngine )

# Generate Manifest Header for Basic Resource Load
find_package(Python3 REQUIRED COMPONENTS Interpreter)
set(MANIFEST_GENERATOR "${CMAKE_SOURCE_DIR}/scripts/ManifestHeaderGenerator.py")
set(RESOURCE_EXAMPLE_MANIFEST_JSON "${CMAKE_CURRENT_SOURCE_DIR}/basic_resource_load/ResourceExampleManifest.json")
set(RESOURCE_EXAMPLE_MANIFEST_HEADER "${CMAKE_CURRENT_SOURCE_DIR}/basic_resource_load/src/Manifest.h")

add_custom_command(
    OUTPUT Manifest.h
    COMMAND ${Python3_EXECUTABLE} 
        ${MANIFEST_GENERATOR}
        --files ${RESOURCE_EXAMPLE_MANIFEST_JSON}
        --out ${RESOURCE_EXAMPLE_MANIFEST_HEADER}

    DEPENDS "${MANIFEST_GENERATOR}" "${RESOURCE_EXAMPLE_MANIFEST_JSON}"
    COMMENT "Generating Manifest header file..."
    VERBATIM
)
add_custom_target(BasicResourceLoadExample_Manifest ALL DEPENDS Manifest.h)

add_executable (BasicResourceLoadExample "basic_resource_load/main.cpp" "basic_resource_load/src/ExampleResourcesGameState.cpp" )
add_dependencies(SmasherEngine BasicResourceLoadExample_Manifest)
target_compile_features(BasicResourceLoadExample PUBLIC cxx_std_20)
target_include_directories (BasicResourceLoadExample PUBLIC "basic_resource_load/include" "basic_resource_load/src")
target_link_libraries( BasicResourceLoadExample PRIVATE SmasherEngine )
