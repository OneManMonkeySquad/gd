set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


add_library(foundation STATIC
    code/foundation/foundation.ixx
    code/foundation/global_exception_handler.ixx
    code/foundation/print.ixx
    code/foundation/math.ixx
    code/foundation/plugin_manager.ixx
    code/foundation/api_registry.ixx
    code/foundation/FileWatch.hpp)
target_link_libraries(foundation PUBLIC SDL3::SDL3 EnTT::EnTT)
target_compile_options(foundation PRIVATE /source-charset:utf-8 /execution-charset:utf-8)


add_library(plugin_engine
    code/engine/plugin_engine.cpp)
target_link_libraries(plugin_engine PRIVATE foundation)
target_compile_options(plugin_engine PRIVATE /source-charset:utf-8 /execution-charset:utf-8)


add_library(plugin_sprite_manager
    code/sprite_manager/plugin_sprite_manager.cpp)
target_link_libraries(plugin_sprite_manager PRIVATE foundation SDL3_image::SDL3_image)
target_compile_options(plugin_sprite_manager PRIVATE /source-charset:utf-8 /execution-charset:utf-8)
