cmake_minimum_required(VERSION 3.8)
project(edge_scheduler_ros2)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic -std=c++17)
endif()

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(std_msgs REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(cv_bridge REQUIRED)

# Include EdgeModelScheduler Core headers
include_directories(
  include
  ../../../include
)

# Core EdgeModelScheduler sources
set(SCHEDULER_SOURCES
  ../../../src/ResourceMonitor.cpp
  ../../../src/Scheduler.cpp
  ../../../src/AdvancedScheduler.cpp
  ../../../src/TensorRTRunner.cpp
)

# ROS 2 Obstacle Arbiter Node
add_executable(obstacle_arbiter_node
  src/obstacle_arbiter_node.cpp
  ${SCHEDULER_SOURCES}
)

ament_target_dependencies(obstacle_arbiter_node
  rclcpp
  sensor_msgs
  std_msgs
  geometry_msgs
  cv_bridge
)

target_link_libraries(obstacle_arbiter_node pthread)

install(TARGETS
  obstacle_arbiter_node
  DESTINATION lib/${PROJECT_NAME}
)

install(DIRECTORY launch
  DESTINATION share/${PROJECT_NAME}
)

ament_package()
