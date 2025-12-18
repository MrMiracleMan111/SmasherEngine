cmake_minimum_required (VERSION 3.8)
project ("SmasherEngine")
include(GenerateExportHeader)
include(FetchContent)

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

set (GENERATE_DOCS TRUE CACHE BOOL "Generate Doxygen Documentation Files")
set (SMASHER_EXAMPLES TRUE CACHE BOOL "Should examples be compiled")
set (SMASHER_STRESS_TEST TRUE CACHE BOOL "Should stress test be compiled")
set (SMASHER_TESTS TRUE CACHE BOOL "Should unit tests be compiled")
set (SMASHER_BENCHMARK FALSE CACHE BOOL "Should benchmarking info be logged")
set (SMASHER_CATCH_EXCEPTIONS TRUE CACHE BOOL "Should Smasher Engine catch all Exceptions. This can make debugging difficult if your debugger struggles to track propagated exceptions.'")
set(MANIFEST_GENERATOR "${CMAKE_SOURCE_DIR}/scripts/ManifestHeaderGenerator.py")
if(NOT DEFINED SMASHER_BENCHMARK_ENGINE_OUTPUT_DIR)
    message(STATUS "Using Default Output Directoy ${CMAKE_BINARY_DIR} for Smasher")
    set(SMASHER_ENGINE_OUTPUT_DIR ${CMAKE_BINARY_DIR})
else()
    message(STATUS "Using Output Directoy ${SMASHER_BENCHMARK_ENGINE_OUTPUT_DIR} for Smasher")
endif()
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(SMASHER_TESTS)
# GoogleTest v1.17.0
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG 52eb8108c5bdec04579160ae17225d66034bd723
    )
    # For Windows: Prevent overriding the parent project's compiler/linker settings
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    
    FetchContent_MakeAvailable(googletest)
    
    if(NOT TARGET gtest OR NOT TARGET gtest_main)
        message(FATAL_ERROR "Could not Find GTest Targets")
    endif()

    set_target_properties(gtest gtest_main PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY ${SMASHER_ENGINE_OUTPUT_DIR}
    LIBRARY_OUTPUT_DIRECTORY ${SMASHER_ENGINE_OUTPUT_DIR}
    RUNTIME_OUTPUT_DIRECTORY ${SMASHER_ENGINE_OUTPUT_DIR})
    enable_testing()
    include(GoogleTest)
endif()

# Box2D v3.1.1
FetchContent_Declare(
    box2d
    GIT_REPOSITORY https://github.com/erincatto/box2d.git
    GIT_TAG 8c661469c9507d3ad6fbd2fea3f1aa71669c2fe3
)

set(TMP_BUILD_SHARED_LIBS BUILD_SHARED_LIBS)
set(BUILD_SHARED_LIBS ON)
set(DBOX2D_BUILD_DOCS OFF)
set(DGLFW_BUILD_WAYLAND OFF)
FetchContent_MakeAvailable(box2d)
set_target_properties(box2d PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY ${SMASHER_ENGINE_OUTPUT_DIR}
    LIBRARY_OUTPUT_DIRECTORY ${SMASHER_ENGINE_OUTPUT_DIR}
    RUNTIME_OUTPUT_DIRECTORY ${SMASHER_ENGINE_OUTPUT_DIR}
)
set(BUILD_SHARED_LIBS TMP_BUILD_SHARED_LIBS)

# Verify box2D target exists
if(NOT TARGET box2d)
    message(FATAL_ERROR "Failed to fetch/build Box2D")
endif()

# SFML 2.6.2 69ea0cd863aed1d4092b970b676924a716ff718b
set(SFML_VERSION "2.6.1")
set(TMP_BUILD_SHARED_LIBS BUILD_SHARED_LIBS)
FetchContent_Declare(
    sfml
    GIT_REPOSITORY https://github.com/SFML/SFML.git
    GIT_TAG "${SFML_VERSION}"
)
set(BUILD_SHARED_LIBS ON)
set(SFML_BUILD_EXAMPLES OFF)
set(SFML_BUILD_DOC OFF)

FetchContent_MakeAvailable(sfml)
list(APPEND SFML_MODULE_NAMES main window system graphics audio network)
foreach(module_name IN LISTS SFML_MODULE_NAMES)
    if(NOT TARGET sfml-${module_name})
        message(FATAL_ERROR "Missing SFML ${SFML_VERSION} System ${module_name}")
    endif()
    set_target_properties(sfml-${module_name} PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY ${SMASHER_ENGINE_OUTPUT_DIR}
    LIBRARY_OUTPUT_DIRECTORY ${SMASHER_ENGINE_OUTPUT_DIR}
    RUNTIME_OUTPUT_DIRECTORY ${SMASHER_ENGINE_OUTPUT_DIR})
    message(STATUS "Configured SFML Module ${module_name}")
endforeach()

set(BUILD_SHARED_LIBS TMP_BUILD_SHARED_LIBS)

message(STATUS "Fetched SFML Successfully")



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

# Compile Engine
add_library(SmasherEngine SHARED
"src/Smasher/Core.cpp"
"include/Smasher/Core.h"
"include/Smasher/Layer.h"
"include/Smasher/UUID.h"
"src/Smasher/UUID.cpp"
"include/Smasher/Base.h"
"src/Smasher/Layer.cpp"
"include/Smasher/Engine.h"
"src/Smasher/Engine.cpp"
"include/Smasher/Exceptions.h"
"include/Smasher/EventManager.h"
"src/Smasher/EventManager.cpp"
"include/Smasher/Events.h"
"include/Smasher/ComponentManagers/GenericComponentManager.h"
"include/Smasher/IComponent.h"
"include/Smasher/Entity.h"
"src/Smasher/Entity.cpp"
"include/Smasher/Components/Transform2DComponent.h"
"include/Smasher/EngineConfig.h"
"include/Smasher/ResourceManager.h"
"src/Smasher/ResourceManager.cpp"
"include/Smasher/IComponentManager.h"
"include/Smasher/ComponentManagers/BaseComponentManager.h"
"include/Smasher/ComponentManagers/BaseComponentManager.inl"
"include/Smasher/Components/DrawableComponent.h"
"src/Smasher/Components/DrawableComponent.cpp"
"include/Smasher/Resources.h"
"include/Smasher/Components/TextComponent.h"
"src/Smasher/Components/TextComponent.cpp"
"include/Smasher/ComponentManagers/DrawableComponentManager.h"
"src/Smasher/ComponentManagers/DrawableComponentManager.cpp"
"include/Smasher/ComponentManagers/RenderBatch.h"
"src/Smasher/ComponentManagers/RenderBatch.cpp"
"src/Smasher/Base.cpp"
"src/Smasher/Components/Transform2DComponent.cpp"
"include/Smasher/Components/Transform2DWrapper.h"
"include/Smasher/Components/Transform2DWrapper.inl"
"include/Smasher/EventFeeder.h"
"src/Smasher/EventFeeder.cpp"
"include/Smasher/Components/CameraComponent.h"
"src/Smasher/Components/CameraComponent.cpp"
"include/Smasher/ComponentManagers/GenericComponentManager.inl"
"include/Smasher/ISerializeable.h"
"include/Smasher/Components/UIPanelComponent.h"
"src/Smasher/Components/UIPanelComponent.cpp"
"src/Smasher/IComponent.cpp"
"include/Smasher/LayerTransition.h"
"include/Smasher/ComponentManagers/UIPanelComponentManager.h"
"src/Smasher/ComponentManagers/UIPanelComponentManager.cpp"
"include/Smasher/Interpolation.h"
"src/Smasher/Interpolation.cpp"
"include/Smasher/Drawable.h"
"include/Smasher/ComponentManagers/PhysicsComponentManager.h"
"src/Smasher/ComponentManagers/PhysicsComponentManager.cpp"
"include/Smasher/Components/PhysicsComponent.h"
"src/Smasher/Components/PhysicsComponent.cpp"
"include/Smasher/PhysicsManager.h"
"src/Smasher/PhysicsManager.cpp"
"include/Smasher/Physics.h")

# Try to include machine-specific config if it exists
if(EXISTS "${CMAKE_SOURCE_DIR}/local.cmake")
    message(STATUS "Including local configuration from local.cmake")
    include("${CMAKE_SOURCE_DIR}/local.cmake")
endif()

find_package(OpenGL REQUIRED)
# find_package(SFML 2.6.2 COMPONENTS system network window graphics audio REQUIRED)
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

if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/include/Smasher/ComponentManagers")
    message(SEND_ERROR "Could not find ComponentManagers include directory")
else()
    message(STATUS "Found ComponentManagers Directory: ${CMAKE_CURRENT_SOURCE_DIR}/include/Smasher/ComponentManagers")
endif()

if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/include/Smasher/ComponentManagers")
    message(SEND_ERROR "Could not find ComponentManagers include directory")
else()
    message(STATUS "Found ComponentManagers Directory: ${CMAKE_CURRENT_SOURCE_DIR}/include/Smasher/ComponentManagers")
endif()

if(WIN32)
    set(SFML_DYNAMIC_LIB_FILE_EXTENSION ".dll")
elseif(UNIX)
    set(SFML_DYNAMIC_LIB_FILE_EXTENSION ".so")
else()
    message(SEND_ERROR "Delete this CMake line and copy over the SFML shared libraries manually")
endif()

if(GENERATE_DOCS)
    find_package(Doxygen)
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

target_link_libraries(SmasherEngine PRIVATE GTest::gtest_main)
target_link_libraries(SmasherEngine PUBLIC
sfml-audio
sfml-window
sfml-graphics
sfml-network
sfml-system
${GLEW_TARGET}
OpenGL::GL
OpenGL::GLU
box2d::box2d)


target_compile_features(SmasherEngine PUBLIC cxx_std_20)

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET SmasherEngine PROPERTY CXX_STANDARD 20)
endif()

# TODO: Add tests and install targets if needed.

# Compile Examples

if(SMASHER_EXAMPLES)
    add_subdirectory("examples")
endif()

if(SMASHER_STRESS_TEST)
    add_subdirectory("stress_test")
endif()

# Compile Tests

if(SMASHER_TESTS)
    add_subdirectory("tests")
endif()

if(SMASHER_BENCHMARK)
    target_compile_definitions(SmasherEngine PUBLIC BENCHMARK)
endif()

if(SMASHER_CATCH_EXCEPTIONS)
    target_compile_definitions(SmasherEngine PUBLIC CATCH_EXCEPTIONS)
endif()
