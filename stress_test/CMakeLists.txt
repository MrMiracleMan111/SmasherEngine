# Generate Manifest Header for Basic Resource Load
find_package(Python3 REQUIRED COMPONENTS Interpreter)
set(STRESS_TEST_MANIFEST_JSON "${CMAKE_CURRENT_SOURCE_DIR}/Manifest.json")
set(STRESS_TEST_MANIFEST_HEADER "${CMAKE_CURRENT_SOURCE_DIR}/Manifest.h")

add_custom_command(
    OUTPUT ${STRESS_TEST_MANIFEST_HEADER}
    COMMAND ${Python3_EXECUTABLE} 
        ${MANIFEST_GENERATOR}
        --files ${STRESS_TEST_MANIFEST_JSON}
        --out ${STRESS_TEST_MANIFEST_HEADER}
        --resources "${CMAKE_CURRENT_SOURCE_DIR}"

    DEPENDS "${MANIFEST_GENERATOR}" "${STRESS_TEST_MANIFEST_JSON}"
    COMMENT "Generating Manifest header file..."
    VERBATIM
)

# Basic Resource Load Example
add_custom_target(generate_manifest_SmasherStressTest DEPENDS ${STRESS_TEST_MANIFEST_HEADER})
add_executable (SmasherStressTest "main.cpp" "include/BallComponent.h" "src/BallComponent.cpp" "include/StressTestLayer.h" "src/StressTestLayer.cpp")

set_target_properties(SmasherStressTest PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG "${SMASHER_ENGINE_OUTPUT_DIR}")
set_target_properties(SmasherStressTest PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE "${SMASHER_ENGINE_OUTPUT_DIR}")
add_dependencies(SmasherStressTest generate_manifest_SmasherStressTest)
target_compile_features(SmasherStressTest PUBLIC cxx_std_20)
target_include_directories (SmasherStressTest PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}" "include" "src")
target_link_libraries( SmasherStressTest PRIVATE SmasherEngine )
