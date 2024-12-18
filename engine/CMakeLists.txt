set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


add_library(foundation STATIC
    src/foundation/foundation.ixx
    src/foundation/global_exception_handler.ixx
    src/foundation/print.ixx
    src/foundation/math.ixx
    src/foundation/plugin_manager.ixx
    src/foundation/api_registry.ixx
    src/foundation/FileWatch.hpp)
target_link_libraries(foundation PUBLIC SDL3::SDL3 EnTT::EnTT)
target_compile_options(foundation PRIVATE /source-charset:utf-8 /execution-charset:utf-8)



add_library(plugin_engine
    src/engine/plugin_engine.cpp)
target_link_libraries(plugin_engine PRIVATE foundation)
target_compile_options(plugin_engine PRIVATE /source-charset:utf-8 /execution-charset:utf-8)
set_target_properties(plugin_engine PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")


add_library(plugin_job_system
    src/job_system/plugin_job_system.cpp)
target_link_libraries(plugin_job_system PRIVATE foundation)
target_compile_options(plugin_job_system PRIVATE /source-charset:utf-8 /execution-charset:utf-8)
set_target_properties(plugin_job_system PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")


add_library(plugin_sprite_manager
    src/sprite_manager/plugin_sprite_manager.cpp)
target_link_libraries(plugin_sprite_manager PRIVATE foundation SDL3_image::SDL3_image)
target_compile_options(plugin_sprite_manager PRIVATE /source-charset:utf-8 /execution-charset:utf-8)
set_target_properties(plugin_sprite_manager PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")


set_target_properties(
    foundation
    plugin_engine
    plugin_job_system
    plugin_sprite_manager
    PROPERTIES FOLDER "Engine")