# Basic Resource Load Example
add_executable (SmasherStressTest "main.cpp" "include/BallComponent.h" "src/BallComponent.cpp" "include/StressTestLayer.h" "src/StressTestLayer.cpp" "include/StatisticsLayer.h" "src/StatisticsLayer.cpp")

set_target_properties(SmasherStressTest PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG "${SMASHER_ENGINE_OUTPUT_DIR}")
set_target_properties(SmasherStressTest PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE "${SMASHER_ENGINE_OUTPUT_DIR}")
add_dependencies(SmasherStressTest generate_manifest_SMASHER)
target_compile_features(SmasherStressTest PUBLIC cxx_std_20)
target_include_directories (SmasherStressTest PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}" "include" "src")
target_link_libraries( SmasherStressTest PRIVATE SmasherEngine )
