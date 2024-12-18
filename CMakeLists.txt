cmake_minimum_required(VERSION 3.31)

project(gd)

SET(SDLIMAGE_AVIF OFF CACHE BOOL "Support loading AVIF images" FORCE)
SET(SDLIMAGE_TIF OFF CACHE BOOL "Support loading TIFF images" FORCE)
SET(SDLIMAGE_WEBP OFF CACHE BOOL "Support loading WEBP images" FORCE)

add_subdirectory(deps/SDL3 EXCLUDE_FROM_ALL)
add_subdirectory(deps/SDL3_image EXCLUDE_FROM_ALL)
add_subdirectory(deps/entt EXCLUDE_FROM_ALL)
add_subdirectory(deps/json EXCLUDE_FROM_ALL)
add_subdirectory(engine)
add_subdirectory(games/flappy)

set_target_properties(SDL3-shared PROPERTIES FOLDER "Deps")
set_target_properties(SDL_uclibc PROPERTIES FOLDER "Deps")
set_target_properties(SDL3_image-shared PROPERTIES FOLDER "Deps")



# add_custom_target(copy_files ALL)

# add_custom_command(
#     TARGET copy_files POST_BUILD
#     COMMAND "${CMAKE_COMMAND}" -E make_directory ${CMAKE_BINARY_DIR}/bin
#     VERBATIM)
# add_custom_command(
#     TARGET copy_files POST_BUILD
#     COMMAND "${CMAKE_COMMAND}" -E copy $<TARGET_FILE:SDL3::SDL3-shared> $<TARGET_PDB_FILE:SDL3-shared> ${CMAKE_BINARY_DIR}/bin/$<CONFIG>
#     VERBATIM)
# add_custom_command(
#     TARGET copy_files POST_BUILD
#     COMMAND "${CMAKE_COMMAND}" -E copy $<TARGET_FILE:SDL3_image::SDL3_image> $<TARGET_PDB_FILE:SDL3_image::SDL3_image> ${CMAKE_BINARY_DIR}/bin/$<CONFIG>
#     VERBATIM)
# add_custom_command(
#     TARGET copy_files POST_BUILD
#     COMMAND "${CMAKE_COMMAND}" -E copy $<TARGET_FILE:plugin_engine> $<TARGET_PDB_FILE:plugin_engine> ${CMAKE_BINARY_DIR}/bin/$<CONFIG>
#     VERBATIM)
# add_custom_command(
#     TARGET copy_files POST_BUILD
#     COMMAND "${CMAKE_COMMAND}" -E copy $<TARGET_FILE:plugin_job_system> $<TARGET_PDB_FILE:plugin_job_system> ${CMAKE_BINARY_DIR}/bin/$<CONFIG>
#     VERBATIM)
# add_custom_command(
#     TARGET copy_files POST_BUILD
#     COMMAND "${CMAKE_COMMAND}" -E copy $<TARGET_FILE:plugin_sprite_manager> $<TARGET_PDB_FILE:plugin_sprite_manager> ${CMAKE_BINARY_DIR}/bin/$<CONFIG>
#     VERBATIM)
# add_custom_command(
#     TARGET copy_files POST_BUILD
#     COMMAND "${CMAKE_COMMAND}" -E copy $<TARGET_FILE:plugin_game> $<TARGET_PDB_FILE:plugin_game> ${CMAKE_BINARY_DIR}/bin/$<CONFIG>
#     VERBATIM)
# # add_custom_command(
# #     TARGET copy_files POST_BUILD
# #     COMMAND "${CMAKE_COMMAND}" -E copy $<TARGET_FILE:flappy> $<TARGET_PDB_FILE:flappy> ${CMAKE_BINARY_DIR}/bin
# #     VERBATIM)
# add_custom_command(
#     TARGET copy_files POST_BUILD
#     COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/games/flappy/assets ${CMAKE_BINARY_DIR}/bin/$<CONFIG>/assets)