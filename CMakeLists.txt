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