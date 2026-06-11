cmake_minimum_required(VERSION 3.8...3.22)
project(pgm_map_creator)

set(CMAKE_CXX_STANDARD 17)

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)

include(FindPkgConfig)
if(PKG_CONFIG_FOUND)
  pkg_check_modules(GAZEBO gazebo)
  pkg_check_modules(SDF sdformat)
endif()

find_package(Boost REQUIRED COMPONENTS system)

include_directories(
  ${GAZEBO_INCLUDE_DIRS}
  ${SDF_INCLUDE_DIRS}
  ${CMAKE_CURRENT_BINARY_DIR}/msgs
)

link_directories(${GAZEBO_LIBRARY_DIRS} ${CMAKE_CURRENT_BINARY_DIR}/msgs)

add_subdirectory(msgs)

add_executable(request_publisher src/request_publisher.cc)
target_link_libraries(request_publisher
  collision_map_creator_msgs
  ${Boost_LIBRARIES}
  ${GAZEBO_LIBRARIES}
  ${SDF_LIBRARIES}
)
add_dependencies(request_publisher collision_map_creator_msgs)

add_library(collision_map_creator SHARED src/collision_map_creator.cc)
target_link_libraries(collision_map_creator
  collision_map_creator_msgs
  ${Boost_LIBRARIES}
  ${GAZEBO_LIBRARIES}
  ${SDF_LIBRARIES}
)
add_dependencies(collision_map_creator collision_map_creator_msgs)

install(TARGETS request_publisher collision_map_creator
  DESTINATION lib/${PROJECT_NAME}
)

install(DIRECTORY launch world maps
  DESTINATION share/${PROJECT_NAME}
)

ament_package()
