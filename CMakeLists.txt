cmake_minimum_required(VERSION 3.31)

project(gd)

add_subdirectory(deps/SDL3 EXCLUDE_FROM_ALL)
add_subdirectory(deps/entt)
add_subdirectory(games/flappy)