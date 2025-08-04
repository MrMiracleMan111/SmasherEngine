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

set (SFML_INCLUDE_DIR CACHE FILEPATH "Include directory for SFML version 2.6.2")
set (SFML_LIB_DIR CACHE FILEPATH "Library directory for SFML version 2.6.2")
set (GLEW_PATH CACHE FILEPATH "Path to GLEW (used by FindPackage tool) for example C:/Users/Foo/Documents/glew, this project was develoepd with GLEW 2.1.0")
set (GLEW_LIB_PATHS CACHE FILEPATH "Path to folder containing GLEW libraries (for recent versions glew/lib/ is not specific enough, you need to use something like glew/lib/Release/x64/. This project was develoepd with GLEW 2.1.0)")
set (GOOGLE_TEST_INCLUDE_DIR FILEPATH "Include directory for Google's GTest tool'")
set (SMASHER_EXAMPLES TRUE CACHE BOOL "Should examples be compiled")
set (SMASHER_STRESS_TEST TRUE CACHE BOOL "Should stress test be compiled")
set (SMASHER_TESTS TRUE CACHE BOOL "Should unit tests be compiled")
# Try to include machine-specific config if it exists
if(EXISTS "${CMAKE_SOURCE_DIR}/local.cmake")
    message(STATUS "Including local configuration from local.cmake")
    include("${CMAKE_SOURCE_DIR}/local.cmake")
endif()

find_package(OpenGL REQUIRED)

include_directories(${OPENGL_INCLUDE_DIRS})

list(APPEND CMAKE_PREFIX_PATH "${GLEW_PATH}")
# list(APPEND CMAKE_LIBRARY_PATH "${GLEW_LIB_PATHS}")

find_package(GLEW REQUIRED)

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

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(SFML_DYNAMIC_LIB_SUFFIX "-d")
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
    set(SFML_DYNAMIC_LIB_SUFFIX "")
    set(NDEBUG 1)
else()
    message(WARNING "Unknown Build Type assuming Debug")
    set(SFML_DYNAMIC_LIB_SUFFIX "-d")
endif()

if(WIN32)
    set(SFML_DYNAMIC_LIB_FILE_EXTENSION ".dll")
elseif(UNIX)
    set(SFML_DYNAMIC_LIB_FILE_EXTENSION ".so")
else()
    message(SEND_ERROR "Delete this CMake line and copy over the SFML shared libraries manually")
endif()

set(SFML_DYNAMIC_LIB_LIST
    "${SFML_LIB_TARGET_DIR}/sfml-audio${SFML_DYNAMIC_LIB_SUFFIX}-2${SFML_DYNAMIC_LIB_FILE_EXTENSION}"
    "${SFML_LIB_TARGET_DIR}/sfml-graphics${SFML_DYNAMIC_LIB_SUFFIX}-2${SFML_DYNAMIC_LIB_FILE_EXTENSION}"
    "${SFML_LIB_TARGET_DIR}/sfml-network${SFML_DYNAMIC_LIB_SUFFIX}-2${SFML_DYNAMIC_LIB_FILE_EXTENSION}"
    "${SFML_LIB_TARGET_DIR}/sfml-system${SFML_DYNAMIC_LIB_SUFFIX}-2${SFML_DYNAMIC_LIB_FILE_EXTENSION}"
    "${SFML_LIB_TARGET_DIR}/sfml-window${SFML_DYNAMIC_LIB_SUFFIX}-2${SFML_DYNAMIC_LIB_FILE_EXTENSION}"
)

foreach(DYNAMIC_LIB ${SFML_DYNAMIC_LIB_LIST})
    message(STATUS "Copying ${DYNAMIC_LIB} into ${CMAKE_BINARY_DIR}")
    file(COPY "${DYNAMIC_LIB}" DESTINATION "${CMAKE_BINARY_DIR}/")
endforeach()

# Compile Engine
add_library(SmasherEngine SHARED "Core.cpp" "Core.h" "include/GameState.h" "include/UUID.h" "src/UUID.cpp" "include/Base.h" "src/GameState.cpp" "include/Engine.h" "src/Engine.cpp" "include/Exceptions.h" "include/EventManager.h" "src/EventManager.cpp" "include/Events.h"  "ComponentManagers/GenericComponentManager.h" "include/IComponent.h"  "include/Entity.h" "src/Entity.cpp"   "Components/Transform2DComponent.h" "include/EngineConfig.h"   "include/ResourceManager.h" "include/IComponentManager.h" "ComponentManagers/BaseComponentManager.h"  "Components/DrawableComponent.h" "Components/DrawableComponent.cpp" "include/Resources.h" "Components/TextComponent.h" "Components/TextComponent.cpp" "ComponentManagers/DrawableComponentManager.h" "ComponentManagers/DrawableComponentManager.cpp" "ComponentManagers/RenderBatch.h" "ComponentManagers/RenderBatch.cpp" "src/Base.cpp")

generate_export_header(SmasherEngine EXPORT_FILE_NAME "${CMAKE_BINARY_DIR}/exports/Smasher_export.h")
target_include_directories(SmasherEngine PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/include" "${CMAKE_CURRENT_SOURCE_DIR}/ComponentManagers" "${SFML_INCLUDE_DIR}" "${CMAKE_BINARY_DIR}/exports")
target_link_directories(SmasherEngine PUBLIC "${SFML_LIB_TARGET_DIR}")

target_link_libraries(SmasherEngine PUBLIC
"sfml-audio${SFML_DYNAMIC_LIB_SUFFIX}"
"sfml-window${SFML_DYNAMIC_LIB_SUFFIX}"
"sfml-graphics${SFML_DYNAMIC_LIB_SUFFIX}"
"sfml-network${SFML_DYNAMIC_LIB_SUFFIX}"
"sfml-system${SFML_DYNAMIC_LIB_SUFFIX}"
GLEW::GLEW)
target_link_libraries(SmasherEngine PUBLIC ${OPENGL_LIBRARIES})

target_compile_features(SmasherEngine PUBLIC cxx_std_20)

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET SmasherEngine PROPERTY CXX_STANDARD 20)
endif()

# TODO: Add tests and install targets if needed.

# Compile Examples

if(DEFINED SMASHER_EXAMPLES)
    add_subdirectory("examples")
endif()

# Compile Tests

if(DEFINED SMASHER_TESTS)
    add_subdirectory("tests")
endif()
