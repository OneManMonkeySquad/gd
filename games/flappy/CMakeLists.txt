set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)



add_library(foundation STATIC
    code/foundation/foundation.ixx)

target_link_libraries(foundation
    PUBLIC SDL3::SDL3)



add_library(plugin_engine
    code/engine/plugin_engine.cpp)

target_link_libraries(plugin_engine
    PRIVATE foundation)


add_library(plugin_sprite_manager
    code/sprite_manager/plugin_sprite_manager.cpp)

target_link_libraries(plugin_sprite_manager
    PRIVATE foundation SDL3_image::SDL3_image)


add_library(plugin_game
    code/game/plugin_game.cpp)

target_link_libraries(plugin_game
    PRIVATE foundation EnTT::EnTT)




add_executable(flappy
    code/main.cpp
    )

target_compile_definitions(flappy PRIVATE WIN32_LEAN_AND_MEAN NOMINMAX)
target_link_options(flappy PRIVATE "/SUBSYSTEM:WINDOWS" "/ENTRY:mainCRTStartup")

target_link_libraries(flappy
    PRIVATE foundation)

add_custom_command(TARGET flappy POST_BUILD COMMAND ${CMAKE_COMMAND}
    -E copy_if_different "$<TARGET_FILE:SDL3::SDL3>" $<TARGET_FILE_DIR:flappy>)
add_custom_command(TARGET flappy POST_BUILD COMMAND ${CMAKE_COMMAND}
    -E copy_if_different "$<TARGET_FILE:SDL3_image::SDL3_image>" $<TARGET_FILE_DIR:flappy>)

add_custom_command(TARGET flappy POST_BUILD COMMAND ${CMAKE_COMMAND}
    -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/assets $<TARGET_FILE_DIR:flappy>/assets)