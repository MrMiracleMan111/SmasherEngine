add_executable(
    BasicTests
    "BasicTests.cpp"
)

include_directories("C:/Users/JackMed/source/repos/SmasherEngine/ComponentManagers")

# Set output directory to parent folder where SFML DLLs are located
set_target_properties(BasicTests PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
    TIMEOUT 6
)

# Check what got set
get_target_property(actual_includes BasicTests INCLUDE_DIRECTORIES)
message(STATUS "Target includes: ${actual_includes}")

# Check the compile flags that will actually be used
get_target_property(compile_options BasicTests COMPILE_OPTIONS)
message(STATUS "Compile options: ${compile_options}")

if (NOT TARGET GTest::gtest)
    message(SEND_ERROR "Gtest::gtest target is missing")
endif()

if (NOT TARGET GTest::gtest_main)
    message(SEND_ERROR "Gtest::gtest_main target is missing")
endif()

target_link_libraries(
    BasicTests
    SmasherEngine
    GTest::gtest
    GTest::gtest_main
)

gtest_discover_tests(BasicTests)