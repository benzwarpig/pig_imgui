include_directories(${CMAKE_CURRENT_SOURCE_DIR}
  ./inc
)

link_libraries("-lpthread -lm")

add_definitions(-DMY_MACRO="${CMAKE_CURRENT_SOURCE_DIR}")

file(GLOB_RECURSE PROJECT_SRC "src/*.cpp")

add_executable(${PROJECT_NAME}_opencv_node
  ${PROJECT_SRC}
)

target_link_libraries(${PROJECT_NAME}_opencv_node
  ${catkin_LIBRARIES}
  lib::zemb
)

target_include_directories(${PROJECT_NAME}_opencv_node
  PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/inc
)
