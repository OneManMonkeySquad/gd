set(CMAKE_CXX_STANDARD 20)

add_executable(flappy main.cpp)

target_link_libraries(flappy PRIVATE SDL3::SDL3)

add_custom_command(TARGET flappy POST_BUILD
     COMMAND ${CMAKE_COMMAND} -E copy_if_different
         "$<TARGET_FILE:SDL3::SDL3>"
         $<TARGET_FILE_DIR:flappy>)

add_custom_command(TARGET flappy POST_BUILD
     COMMAND ${CMAKE_COMMAND} -E copy_directory
         ${CMAKE_CURRENT_SOURCE_DIR}/data
         $<TARGET_FILE_DIR:flappy>/data)