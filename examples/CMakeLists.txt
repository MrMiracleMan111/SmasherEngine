# Basic Window Example
add_executable (BasicWindowExample "basic_window/main.cpp" "basic_window/src/ExampleWindowLayer.cpp"  )
foreach(CONFIG ${CMAKE_CONFIGURATION_TYPES})
    string(TOUPPER ${CMAKE_BUILD_TYPE} CONFIG_UPPER)
    set_target_properties(BasicWindowExample PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY_${CONFIG_UPPER} "${SMASHER_ENGINE_OUTPUT_DIR}"
    )
endforeach()
set_target_properties(BasicWindowExample PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG "${SMASHER_ENGINE_OUTPUT_DIR}")
set_target_properties(BasicWindowExample PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE "${SMASHER_ENGINE_OUTPUT_DIR}")
target_compile_features(BasicWindowExample PUBLIC cxx_std_20)
target_include_directories (BasicWindowExample PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}" "basic_window/include" "basic_window/src")
target_link_libraries( BasicWindowExample PRIVATE SmasherEngine )

# Basic Serialize Example
add_executable (BasicSerializeExample "basic_serialize/main.cpp" "basic_serialize/src/SavedComponent.cpp" "basic_serialize/src/BallComponent.cpp" "basic_serialize/src/SerializeLayer.cpp"  )
foreach(CONFIG ${CMAKE_CONFIGURATION_TYPES})
    string(TOUPPER ${CMAKE_BUILD_TYPE} CONFIG_UPPER)
    set_target_properties(BasicSerializeExample PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY_${CONFIG_UPPER} "${SMASHER_ENGINE_OUTPUT_DIR}"
    )
endforeach()
set_target_properties(BasicSerializeExample PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG "${SMASHER_ENGINE_OUTPUT_DIR}")
set_target_properties(BasicSerializeExample PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE "${SMASHER_ENGINE_OUTPUT_DIR}")
target_compile_features(BasicSerializeExample PUBLIC cxx_std_20)
target_include_directories (BasicSerializeExample PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}" "basic_serialize/include" "basic_serialize/src")
target_link_libraries( BasicSerializeExample PRIVATE SmasherEngine )


# Basic Resource Load Example
add_executable (BasicResourceLoadExample "basic_resource_load/main.cpp" "basic_resource_load/src/ExampleResourcesLayer.cpp"   )

set_target_properties(BasicResourceLoadExample PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG "${SMASHER_ENGINE_OUTPUT_DIR}")
set_target_properties(BasicResourceLoadExample PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE "${SMASHER_ENGINE_OUTPUT_DIR}")
add_dependencies(BasicResourceLoadExample generate_manifest_SMASHER )
target_compile_features(BasicResourceLoadExample PUBLIC cxx_std_20)
target_include_directories (BasicResourceLoadExample PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}" "basic_resource_load/include" "basic_resource_load/src")
target_link_libraries( BasicResourceLoadExample PRIVATE SmasherEngine )

# Multiple Layers Example
add_executable (MultipleLayersExample "multiple_layers/main.cpp" "multiple_layers/src/TopLayer.cpp" "multiple_layers/src/MidLayer.cpp" "multiple_layers/src/BottomLayer.cpp" )

set_target_properties(MultipleLayersExample PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG "${SMASHER_ENGINE_OUTPUT_DIR}")
set_target_properties(MultipleLayersExample PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE "${SMASHER_ENGINE_OUTPUT_DIR}")
add_dependencies(MultipleLayersExample generate_manifest_SMASHER)
target_compile_features(MultipleLayersExample PUBLIC cxx_std_20)
target_include_directories (MultipleLayersExample PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}" "multiple_layers/include" "multiple_layers/src")
target_link_libraries( MultipleLayersExample PRIVATE SmasherEngine )

# Basic AABB Physics Engine Example
add_executable (PhysicsExample "basic_physics/main.cpp" "basic_physics/src/ExamplePhysicsLayer.cpp" "basic_physics/include/BoxControllerComponent.h" "basic_physics/src/BoxControllerComponent.cpp" )

set_target_properties(PhysicsExample PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG "${SMASHER_ENGINE_OUTPUT_DIR}")
set_target_properties(PhysicsExample PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE "${SMASHER_ENGINE_OUTPUT_DIR}")
add_dependencies(PhysicsExample generate_manifest_SMASHER )
target_compile_features(PhysicsExample PUBLIC cxx_std_20)
target_include_directories (PhysicsExample PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}" "basic_physics/include" "basic_physics/src")
target_link_libraries( PhysicsExample PRIVATE SmasherEngine )