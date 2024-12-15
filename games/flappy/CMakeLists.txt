set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)



add_library(plugin_game
    code/game/plugin_game.cpp)
target_link_libraries(plugin_game PRIVATE foundation nlohmann_json::nlohmann_json)
target_compile_options(plugin_game PRIVATE /source-charset:utf-8 /execution-charset:utf-8)



add_executable(flappy
    code/main.cpp)

target_compile_definitions(flappy PRIVATE WIN32_LEAN_AND_MEAN NOMINMAX)
target_link_options(flappy PRIVATE "/SUBSYSTEM:WINDOWS" "/ENTRY:mainCRTStartup")
target_compile_options(flappy PRIVATE /source-charset:utf-8 /execution-charset:utf-8)

target_link_libraries(flappy
    PRIVATE foundation)
set_target_properties(flappy
    PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/$<CONFIG>/bin
)

add_custom_command(TARGET flappy POST_BUILD COMMAND ${CMAKE_COMMAND}
    -E copy_if_different "$<TARGET_FILE:plugin_engine>" "$<TARGET_PDB_FILE:plugin_engine>" $<TARGET_FILE_DIR:flappy>)
add_custom_command(TARGET flappy POST_BUILD COMMAND ${CMAKE_COMMAND}
    -E copy_if_different "$<TARGET_FILE:plugin_sprite_manager>" "$<TARGET_PDB_FILE:plugin_sprite_manager>" $<TARGET_FILE_DIR:flappy>)
add_custom_command(TARGET flappy POST_BUILD COMMAND ${CMAKE_COMMAND}
    -E copy_if_different "$<TARGET_FILE:plugin_game>" "$<TARGET_PDB_FILE:plugin_game>" $<TARGET_FILE_DIR:flappy>)
add_custom_command(TARGET flappy POST_BUILD COMMAND ${CMAKE_COMMAND}
    -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/assets $<TARGET_FILE_DIR:flappy>/assets)