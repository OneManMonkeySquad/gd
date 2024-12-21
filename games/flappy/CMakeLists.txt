set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)



add_library(plugin_game
    src/game/plugin_game.cpp)
target_link_libraries(plugin_game PRIVATE foundation nlohmann_json::nlohmann_json)
target_compile_options(plugin_game PRIVATE /source-charset:utf-8 /execution-charset:utf-8)
set_target_properties(plugin_game PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")


add_executable(flappy
    src/main.cpp)
target_link_libraries(flappy PRIVATE foundation)
target_compile_definitions(flappy PRIVATE WIN32_LEAN_AND_MEAN NOMINMAX)
target_link_options(flappy PRIVATE "/SUBSYSTEM:WINDOWS" "/ENTRY:mainCRTStartup")
target_compile_options(flappy PRIVATE /source-charset:utf-8 /execution-charset:utf-8)
set_target_properties(flappy PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")


add_custom_command(
    TARGET flappy POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/assets ${CMAKE_BINARY_DIR}/bin/$<CONFIG>/assets)
add_custom_command(
    TARGET flappy POST_BUILD
    COMMAND "${CMAKE_COMMAND}" -E copy $<TARGET_FILE:SDL3::SDL3-shared> $<TARGET_PDB_FILE:SDL3-shared> ${CMAKE_BINARY_DIR}/bin/$<CONFIG>)
add_custom_command(
    TARGET flappy POST_BUILD
    COMMAND "${CMAKE_COMMAND}" -E copy $<TARGET_FILE:SDL3_image::SDL3_image> $<TARGET_PDB_FILE:SDL3_image::SDL3_image> ${CMAKE_BINARY_DIR}/bin/$<CONFIG>)