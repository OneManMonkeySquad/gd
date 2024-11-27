set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(flappy main.cpp game.ixx)

target_compile_definitions(flappy PRIVATE WIN32_LEAN_AND_MEAN NOMINMAX)
target_link_options(flappy PRIVATE "/SUBSYSTEM:WINDOWS" "/ENTRY:mainCRTStartup")

target_link_libraries(flappy
    PRIVATE SDL3::SDL3 SDL3_image::SDL3_image EnTT::EnTT)

add_custom_command(TARGET flappy POST_BUILD COMMAND ${CMAKE_COMMAND}
    -E copy_if_different "$<TARGET_FILE:SDL3::SDL3>" $<TARGET_FILE_DIR:flappy>)
add_custom_command(TARGET flappy POST_BUILD COMMAND ${CMAKE_COMMAND}
    -E copy_if_different "$<TARGET_FILE:SDL3_image::SDL3_image>" $<TARGET_FILE_DIR:flappy>)

add_custom_command(TARGET flappy POST_BUILD COMMAND ${CMAKE_COMMAND}
    -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/assets $<TARGET_FILE_DIR:flappy>/assets)