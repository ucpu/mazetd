cmake_minimum_required(VERSION 3.22)
project(mazetd CXX)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/externals/cage/cmake")
include(cage_all)
cage_default_init()
cage_assets_add_data_path("${CMAKE_CURRENT_SOURCE_DIR}/data")

add_subdirectory(externals/cage)

file(GLOB_RECURSE mazetd-sources "sources/*")
add_executable(mazetd ${mazetd-sources})
target_link_libraries(mazetd cage-simple)
cage_ide_category(mazetd mazetd)
cage_ide_sort_files(mazetd)
cage_ide_working_dir_in_place(mazetd)
