if (NOT TARGET GTest::gtest)
    message(SEND_ERROR "Gtest::gtest target is missing")
endif()

if (NOT TARGET GTest::gtest_main)
    message(SEND_ERROR "Gtest::gtest_main target is missing")
endif()

# Generate Manifest Header for Basic Resource Load
find_package(Python3 REQUIRED COMPONENTS Interpreter)
set(STRESS_TEST_MANIFEST_JSON "${CMAKE_CURRENT_SOURCE_DIR}/Manifest.json")
set(STRESS_TEST_MANIFEST_HEADER "${CMAKE_CURRENT_SOURCE_DIR}/Manifest.h")

add_custom_command(
    OUTPUT Manifest.h
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
add_custom_target(generate_manifest_BasicTests DEPENDS Manifest.h)


add_executable(
    BasicTests
    "BasicTests.cpp"
)

add_dependencies(BasicTests generate_manifest_BasicTests)

# Set output directory to parent folder where SFML DLLs are located
set_target_properties(BasicTests PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${SMASHER_ENGINE_OUTPUT_DIR}"
    TIMEOUT 6
)

set_target_properties(BasicTests PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG "${SMASHER_ENGINE_OUTPUT_DIR}" TIMEOUT 6)
set_target_properties(BasicTests PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE "${SMASHER_ENGINE_OUTPUT_DIR}" TIMEOUT 6)

target_link_libraries(
    BasicTests
    SmasherEngine
    GTest::gtest
    GTest::gtest_main
)

add_executable(
    InternalComponentTests
    "InternalComponentTests.cpp"
)

# Set output directory to parent folder where SFML DLLs are located
set_target_properties(InternalComponentTests PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${SMASHER_ENGINE_OUTPUT_DIR}"
    TIMEOUT 6
)

set_target_properties(InternalComponentTests PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG "${SMASHER_ENGINE_OUTPUT_DIR}" TIMEOUT 6)
set_target_properties(InternalComponentTests PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE "${SMASHER_ENGINE_OUTPUT_DIR}" TIMEOUT 6)


target_link_libraries(
    InternalComponentTests
    SmasherEngine
    GTest::gtest
    GTest::gtest_main
)


gtest_discover_tests(BasicTests)
gtest_discover_tests(InternalComponentTests)