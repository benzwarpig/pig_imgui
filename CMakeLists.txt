cmake_minimum_required(VERSION 3.12)
project(pig_monitor)

add_compile_options(-std=c++14)

add_subdirectory(lib)
include_directories(lib)

add_subdirectory(imgui_vis)
add_subdirectory(opencv_vis)
