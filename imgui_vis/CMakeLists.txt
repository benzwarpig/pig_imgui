include_directories(${CMAKE_CURRENT_SOURCE_DIR}
  ./inc
  ./imgui
  ./imgui/backends
  ./inc/Implot
)

# link_directories(
# ${catkin_LIBRARY_DIRS}
# )
link_libraries("-lpthread -lm -lGL -lglfw -lrt -ldl")

add_definitions(-DMY_MACRO="${CMAKE_CURRENT_SOURCE_DIR}")

file(GLOB_RECURSE PROJECT_SRC "src/*.cpp")

add_executable(${PROJECT_NAME}_imgui_node
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

target_link_libraries(${PROJECT_NAME}_imgui_node
  ${catkin_LIBRARIES}
)
