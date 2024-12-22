cmake_minimum_required(VERSION 3.31)

project(gd)

# Async exception handling
string(REPLACE "/EHsc" "/EHa" CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})

SET(SDLIMAGE_AVIF OFF CACHE BOOL "Support loading AVIF images" FORCE)
SET(SDLIMAGE_TIF OFF CACHE BOOL "Support loading TIFF images" FORCE)
SET(SDLIMAGE_WEBP OFF CACHE BOOL "Support loading WEBP images" FORCE)

add_subdirectory(deps/SDL3)
add_subdirectory(deps/SDL3_image)
add_subdirectory(deps/entt)
add_subdirectory(deps/json)
add_subdirectory(engine)
add_subdirectory(games/flappy)
add_subdirectory(games/rpg)

set_target_properties(
    SDL3-shared
    SDL_uclibc
    SDL3_image-shared
    PROPERTIES FOLDER "Deps")
