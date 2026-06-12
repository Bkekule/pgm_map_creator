cmake_minimum_required(VERSION 3.8...3.22)
project(pgm_map_creator)

set(CMAKE_CXX_STANDARD 17)

find_package(ament_cmake REQUIRED)

# Gz Harmonic libraries
find_package(gz-sim8 REQUIRED)
find_package(gz-transport13 REQUIRED)

# Protobuf for custom messages
find_package(Protobuf REQUIRED)

# ─── Custom protobuf messages ───────────────────────────────────────────────
set(PROTO_FILES msgs/collision_map_request.proto)
protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${PROTO_FILES})

add_library(collision_map_creator_msgs SHARED ${PROTO_SRCS})
target_link_libraries(collision_map_creator_msgs ${Protobuf_LIBRARIES})
target_include_directories(collision_map_creator_msgs PUBLIC
  ${CMAKE_CURRENT_BINARY_DIR}
  ${Protobuf_INCLUDE_DIRS}
)

# ─── Shared utility libraries ───────────────────────────────────────────────
include_directories(include ${CMAKE_CURRENT_BINARY_DIR})

# PGM writer (no external dependencies)
add_library(pgm_writer STATIC src/pgm_writer.cc)

# Geometry utilities (depends on gz-sim for ECM access)
add_library(geometry_utils STATIC src/geometry_utils.cc)
target_link_libraries(geometry_utils gz-sim8::gz-sim8)

# Vector parsing (depends only on our own protobuf messages)
add_library(vector_parsing STATIC src/vector_parsing.cc)
target_link_libraries(vector_parsing collision_map_creator_msgs)

# ─── Request publisher executable ───────────────────────────────────────────
add_executable(request_publisher src/request_publisher.cc)
target_link_libraries(request_publisher
  collision_map_creator_msgs
  vector_parsing
  gz-transport13::gz-transport13
)
set_target_properties(request_publisher PROPERTIES
  INSTALL_RPATH "$ORIGIN"
)

# ─── Collision map creator plugin (gz-sim system) ───────────────────────────
add_library(collision_map_creator SHARED src/collision_map_creator.cc)
target_link_libraries(collision_map_creator
  collision_map_creator_msgs
  geometry_utils
  pgm_writer
  gz-sim8::gz-sim8
  gz-transport13::gz-transport13
)

# ─── Install ────────────────────────────────────────────────────────────────
install(TARGETS
  request_publisher
  collision_map_creator
  collision_map_creator_msgs
  DESTINATION lib/${PROJECT_NAME}
)

install(DIRECTORY launch config maps
  DESTINATION share/${PROJECT_NAME}
)

ament_package()
