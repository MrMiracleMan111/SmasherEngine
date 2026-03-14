if (NOT TARGET GTest::gtest)
    message(SEND_ERROR "Gtest::gtest target is missing")
endif()

if (NOT TARGET GTest::gtest_main)
    message(SEND_ERROR "Gtest::gtest_main target is missing")
endif()

# Basic Resource Load Example
add_custom_target(generate_manifest_BasicTests DEPENDS ${STRESS_TEST_MANIFEST_HEADER})


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

add_custom_target(generate_manifest_DemoTests DEPENDS ${STRESS_TEST_MANIFEST_HEADER})

add_executable(
    DemoTests
    "DemoTests.cpp"
)

add_dependencies(DemoTests generate_manifest_SMASHER)

# Set output directory to parent folder where SFML DLLs are located
set_target_properties(DemoTests PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${SMASHER_ENGINE_OUTPUT_DIR}"
    TIMEOUT 6
)

set_target_properties(DemoTests PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG "${SMASHER_ENGINE_OUTPUT_DIR}" TIMEOUT 6)
set_target_properties(DemoTests PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE "${SMASHER_ENGINE_OUTPUT_DIR}" TIMEOUT 6)


target_link_libraries(
    DemoTests
    SmasherEngine
)


gtest_discover_tests(BasicTests)
gtest_discover_tests(InternalComponentTests)
