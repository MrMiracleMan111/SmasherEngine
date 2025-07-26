cmake_minimum_required (VERSION 3.8)

project ("SmasherEngine")
include(GenerateExportHeader)
include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.17.0
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)
include(GoogleTest)

enable_testing()

# GoogleTest requires at least C++17
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Try to include machine-specific config if it exists
if(EXISTS "${CMAKE_SOURCE_DIR}/local.cmake")
    message(STATUS "Including local configuration from local.cmake")
    include("${CMAKE_SOURCE_DIR}/local.cmake")
endif()


if(DEFINED SFML_INCLUDE_DIR)
    message(STATUS "SFML_INCLUDE_DIR " ${SFML_INCLUDE_DIR})
else()
    message(SEND_ERROR "SFML_INCLUDE_DIR is NOT defined.")
endif()

if(DEFINED SFML_LIB_DIR)
    message(STATUS "SFML_LIB_DIR " ${SFML_LIB_DIR})
else()
    message(SEND_ERROR "SFML_LIB_DIR is NOT defined.")
endif()

cmake_path(APPEND SFML_LIB_TARGET_DIR "${SFML_LIB_DIR}" "${CMAKE_BUILD_TYPE}")

if(NOT EXISTS "${SFML_LIB_TARGET_DIR}")
    message(SEND_ERROR "SFML Lib folder is missing ${CMAKE_BUILD_TYPE} subdirectory.")
endif()

message(STATUS "Using SFML Target Library path ${SFML_LIB_TARGET_DIR}")

if(NOT EXISTS "${PROJECT_SOURCE_DIR}/include")
    message(STATUS "${PROJECT_SOURCE_DIR}/include")
    message(SEND_ERROR "Could not find include subdirectory")
endif()

if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/ComponentManagers")
    message(SEND_ERROR "Could not find ComponentManagers")
else()
    message(STATUS "Found ComponentManagers Directory: ${CMAKE_CURRENT_SOURCE_DIR}/ComponentManagers")
endif()

# Copy DLLs into build directory (solution for those who haven't installed SFML')

add_custom_target(copy_sfml_shared_libs ALL)

if(WIN32)
    set(SFML_DLL_LIST
        "${SFML_LIB_TARGET_DIR}/sfml-audio-d-2.dll"
        "${SFML_LIB_TARGET_DIR}/sfml-graphics-d-2.dll"
        "${SFML_LIB_TARGET_DIR}/sfml-network-d-2.dll"
        "${SFML_LIB_TARGET_DIR}/sfml-system-d-2.dll"
        "${SFML_LIB_TARGET_DIR}/sfml-window-d-2.dll"
    )
elseif(UNIX)
    set(SFML_DLL_LIST
        "${SFML_LIB_TARGET_DIR}/sfml-audio-d-2.so"
        "${SFML_LIB_TARGET_DIR}/sfml-graphics-d-2.so"
        "${SFML_LIB_TARGET_DIR}/sfml-network-d-2.so"
        "${SFML_LIB_TARGET_DIR}/sfml-system-d-2.so"
        "${SFML_LIB_TARGET_DIR}/sfml-window-d-2.so"
    )
else()
    message(SEND_ERROR "Delete this CMake line and copy over the SFML shared libraries manually")
endif()

foreach(DLL ${SFML_DLL_LIST})
    message(STATUS "Copying ${DLL} into ${CMAKE_BINARY_DIR}")
    file(COPY "${DLL}" DESTINATION "${CMAKE_BINARY_DIR}/")
endforeach()

# Compile Engine
add_library(SmasherEngine SHARED "Core.cpp" "Core.h" "include/GameState.h" "include/UUID.h" "src/UUID.cpp" "include/Base.h" "src/GameState.cpp" "include/Engine.h" "src/Engine.cpp" "include/Exceptions.h" "include/EventManager.h" "src/EventManager.cpp" "include/Events.h" "include/ComponentManager.h" "ComponentManagers/GenericComponentManager.h" "include/Component.h"  "include/Entity.h" "src/Entity.cpp" "include/IRenderable.h" "ComponentManagers/GenericRenderableComponentManager.h" "Components/Transform2DComponent.h")
add_dependencies(SmasherEngine copy_sfml_shared_libs)
generate_export_header(SmasherEngine EXPORT_FILE_NAME "${CMAKE_BINARY_DIR}/exports/Smasher_export.h")
target_include_directories(SmasherEngine PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/include" "${CMAKE_CURRENT_SOURCE_DIR}/ComponentManagers" "${SFML_INCLUDE_DIR}" "${CMAKE_BINARY_DIR}/exports")
target_link_directories(SmasherEngine PUBLIC "${SFML_LIB_TARGET_DIR}")
target_link_libraries(SmasherEngine PUBLIC "sfml-audio-d" "sfml-window-d" "sfml-graphics-d" "sfml-network-d" "sfml-system-d")
target_compile_features(SmasherEngine PUBLIC cxx_std_20)

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET SmasherEngine PROPERTY CXX_STANDARD 20)
endif()

# TODO: Add tests and install targets if needed.

# Compile Examples

if(DEFINED SMASHER_EXAMPLES)
    add_executable (BasicWindowExample "examples/basic_window/main.cpp" "examples/basic_window/src/ExampleAppGameState.cpp" "ComponentManagers/GenericRenderableComponentManager.h")
    target_compile_features(BasicWindowExample PUBLIC cxx_std_20)
    target_include_directories (BasicWindowExample PUBLIC "examples/basic_window/src")
    target_link_libraries( BasicWindowExample PRIVATE SmasherEngine )
endif()

# Compile Tests

if(DEFINED SMASHER_TESTS)
    add_subdirectory("tests")
endif()
