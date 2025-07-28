if (NOT TARGET GTest::gtest)
    message(SEND_ERROR "Gtest::gtest target is missing")
endif()

if (NOT TARGET GTest::gtest_main)
    message(SEND_ERROR "Gtest::gtest_main target is missing")
endif()

add_executable(
    BasicTests
    "BasicTests.cpp"
)

# Set output directory to parent folder where SFML DLLs are located
set_target_properties(BasicTests PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
    TIMEOUT 6
)

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
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
    TIMEOUT 6
)

target_link_libraries(
    InternalComponentTests
    SmasherEngine
    GTest::gtest
    GTest::gtest_main
)


gtest_discover_tests(BasicTests)
gtest_discover_tests(InternalComponentTests)