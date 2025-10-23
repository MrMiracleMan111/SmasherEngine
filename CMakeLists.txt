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

message(STATUS "Your Compiler is ${CMAKE_CXX_COMPILER_ID}")
if(MSVC)
    add_compile_options(/EHsc)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-fexceptions)
else()
    message(STATUS "You will need to modify the CMakeLists.txt to add compiler flags for exception handling.")
endif()

set (GENERATE_DOCS TRUE CACHE BOOL "Generate Doxygen Documentation Files")
set (SMASHER_EXAMPLES TRUE CACHE BOOL "Should examples be compiled")
set (SMASHER_STRESS_TEST TRUE CACHE BOOL "Should stress test be compiled")
set (SMASHER_TESTS TRUE CACHE BOOL "Should unit tests be compiled")
set(MANIFEST_GENERATOR "${CMAKE_SOURCE_DIR}/scripts/ManifestHeaderGenerator.py")
set(SMASHER_ENGINE_OUTPUT_DIR ${CMAKE_BINARY_DIR})

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Compile Engine
add_library(SmasherEngine SHARED "Core.cpp" "Core.h" "include/Layer.h" "include/UUID.h" "src/UUID.cpp" "include/Smasher/Base.h" "src/Layer.cpp" "include/Smasher/Engine.h" "src/Engine.cpp" "include/Exceptions.h" "include/EventManager.h" "src/EventManager.cpp" "include/Events.h"  "ComponentManagers/GenericComponentManager.h" "include/IComponent.h"  "include/Entity.h" "src/Entity.cpp"   "Components/Transform2DComponent.h" "include/Smasher/EngineConfig.h"   "include/ResourceManager.h" "src/ResourceManager.cpp" "include/IComponentManager.h" "ComponentManagers/BaseComponentManager.h"  "Components/DrawableComponent.h" "Components/DrawableComponent.cpp" "include/Resources.h" "Components/TextComponent.h" "Components/TextComponent.cpp" "ComponentManagers/DrawableComponentManager.h" "ComponentManagers/DrawableComponentManager.cpp" "ComponentManagers/RenderBatch.h" "ComponentManagers/RenderBatch.cpp" "src/Base.cpp" "Components/Transform2DComponent.cpp" "Components/Transform2DWrapper.h"  "Components/Transform2DWrapper.inl" "include/EventFeeder.h" "src/EventFeeder.cpp" "Components/CameraComponent.h" "Components/CameraComponent.cpp" "ComponentManagers/GenericComponentManager.inl" "include/ISerializeable.h" "Components/UIPanelComponent.h" "Components/UIPanelComponent.cpp"  "src/IComponent.cpp" "include/LayerTransition.h" "ComponentManagers/UIPanelComponentManager.h" "ComponentManagers/UIPanelComponentManager.cpp")

# Try to include machine-specific config if it exists
if(EXISTS "${CMAKE_SOURCE_DIR}/local.cmake")
    message(STATUS "Including local configuration from local.cmake")
    include("${CMAKE_SOURCE_DIR}/local.cmake")
endif()

find_package(OpenGL REQUIRED)
find_package(SFML 2.6.2 COMPONENTS system network window graphics audio REQUIRED)
find_package(GLEW NAMES glew GLEW QUIET)

if(GLEW_FOUND)
    if (TARGET GLEW::GLEW)
        message(STATUS "using GLEW::GLEW target")
        target_link_libraries(SmasherEngine PUBLIC GLEW::GLEW)
    else()
        set(GLEW_TARGET, ${GLEW_LIBRARIES})
        target_include_directories(SmasherEngine PRIVATE ${GLEW_INCLUDE_DIRS})
    endif()
    message(STATUS "Found GLEW using find_package")
else()
    message(STATUS "GLEW not found with find_package, trying pkg-config")
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(GLEW REQUIRED glew)
    if (GLEW_FOUND)
        message(STATUS "Found GLEW using pkg-config")
        set(GLEW_TARGET, ${GLEW_LIBRARIES})
        target_compile_options(SmasherEngine PRIVATE ${GLEW_CFLAGS_OTHER})
        target_link_directories(SmasherEngine PRIVATE ${GLEW_LIBRARY_DIRS})
        target_include_directories(SmasherEngine PRIVATE ${GLEW_INCLUDE_DIRS})
    else()
        message(SEND_ERROR "Could not find GLEW")
    endif()
endif()

if(NOT EXISTS "${PROJECT_SOURCE_DIR}/include")
    message(STATUS "${PROJECT_SOURCE_DIR}/include")
    message(SEND_ERROR "Could not find include subdirectory")
endif()

if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/ComponentManagers")
    message(SEND_ERROR "Could not find ComponentManagers")
else()
    message(STATUS "Found ComponentManagers Directory: ${CMAKE_CURRENT_SOURCE_DIR}/ComponentManagers")
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(SFML_DYNAMIC_LIB_SUFFIX "-d")
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
    set(SFML_DYNAMIC_LIB_SUFFIX "")
    set(NDEBUG 1)
else()
    message(WARNING "Unknown Build Type assuming Debug")
    set(SFML_DYNAMIC_LIB_SUFFIX "-d")
    set(CMAKE_BUILD_TYPE "Debug")
endif()

if(WIN32)
    set(SFML_DYNAMIC_LIB_FILE_EXTENSION ".dll")
elseif(UNIX)
    set(SFML_DYNAMIC_LIB_FILE_EXTENSION ".so")
else()
    message(SEND_ERROR "Delete this CMake line and copy over the SFML shared libraries manually")
endif()

find_package(Doxygen)
if(GENERATE_DOCS)
    if (NOT EXISTS "${CMAKE_SOURCE_DIR}/doc")
        file(MAKE_DIRECTORY "${CMAKE_SOURCE_DIR}/doc")
    endif()

    if(DOXYGEN_FOUND)
        set(DOXYGEN_INPUT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
        set(DOXYGEN_OUTPUT_DIR "${CMAKE_SOURCE_DIR}/doc")

        configure_file(
            "${CMAKE_SOURCE_DIR}/Doxygen.in"
            "${CMAKE_SOURCE_DIR}/Doxyfile"
            @ONLY
        )

        add_custom_target(doxygen_docs ALL
            COMMAND ${DOXYGEN_EXECUTABLE} "${CMAKE_SOURCE_DIR}/Doxyfile"
            WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/doc"
            COMMENT "Generating Doxygen files"
            BYPRODUCTS "${DOXYGEN_OUTPUT_DIR}/html/index.html"
        )
    else()
        message(SEND_ERROR "Could not find Doxygen")
    endif()

endif()

set_target_properties(SmasherEngine PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG "${SMASHER_ENGINE_OUTPUT_DIR}")
set_target_properties(SmasherEngine PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE "${SMASHER_ENGINE_OUTPUT_DIR}")

generate_export_header(SmasherEngine EXPORT_FILE_NAME "${CMAKE_BINARY_DIR}/exports/Smasher_export.h")
target_include_directories(SmasherEngine PUBLIC ${OPENGL_INCLUDE_DIR} ${GLEW_INCLUDE_DIRS} "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/include" "${CMAKE_CURRENT_SOURCE_DIR}/ComponentManagers" "${CMAKE_BINARY_DIR}/exports")

target_link_libraries(SmasherEngine PUBLIC
"sfml-audio"
"sfml-window"
"sfml-graphics"
"sfml-network"
"sfml-system"
${GLEW_TARGET}
OpenGL::GL
OpenGL::GLU)

target_compile_features(SmasherEngine PUBLIC cxx_std_20)

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET SmasherEngine PROPERTY CXX_STANDARD 20)
endif()

# TODO: Add tests and install targets if needed.

# Compile Examples

if(DEFINED SMASHER_EXAMPLES)
    add_subdirectory("examples")
endif()

if(DEFINED SMASHER_STRESS_TEST)
    add_subdirectory("stress_test")
endif()

# Compile Tests

if(DEFINED SMASHER_TESTS)
    add_subdirectory("tests")
endif()

if(DEFINED SMASHER_BENCHMARK)
    target_compile_definitions(SmasherEngine PUBLIC BENCHMARK)
endif()
