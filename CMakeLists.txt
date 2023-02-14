cmake_minimum_required(VERSION 3.12)
project(pig_monitor)

add_compile_options(-std=c++14)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}
    ./inc
    ./imgui
    ./imgui/backends
)

# link_directories(
# ${catkin_LIBRARY_DIRS}
# )
link_libraries("-lpthread -lm -lGL -lglfw -lrt -ldl")

file(GLOB_RECURSE PROJECT_SRC "src/*.cpp")

add_executable(${PROJECT_NAME}_node
    ${PROJECT_SRC}
    imgui/imgui.cpp
    imgui/imgui_demo.cpp
    imgui/imgui_draw.cpp
    imgui/imgui_tables.cpp
    imgui/imgui_widgets.cpp
    imgui/backends/imgui_impl_glfw.cpp
    imgui/backends/imgui_impl_opengl2.cpp
    imgui/implot.cpp
    imgui/implot_items.cpp
    imgui/implot_demo.cpp
)

target_link_libraries(${PROJECT_NAME}_node
    ${catkin_LIBRARIES}
)
