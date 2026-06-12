# PGM Map Creator (ROS2 Jazzy + Gz Harmonic)

Generate a 2D PGM occupancy map from a Gazebo world file.

## Overview

This package provides a Gz Sim (Harmonic) world system plugin that reads collision
geometries from the loaded world and rasterizes them into a PGM occupancy grid map.
A companion `request_publisher` executable sends the map generation parameters to
the running simulation via gz-transport.

## Architecture

- **collision_map_creator** — A `gz::sim::System` plugin (shared library) that
  loads into the Gazebo simulation, subscribes to `/collision_map/command`, and
  generates a PGM file by checking collision bounding boxes at each grid cell.
- **request_publisher** — A standalone executable that publishes a
  `CollisionMapRequest` message specifying the map boundaries, resolution, scan
  height, and output filename.

## Dependencies

Built for the **`osrf/ros:jazzy-simulation`** Docker image (Ubuntu 24.04 + ROS2 Jazzy + Gz Harmonic).

Additional packages needed in your Docker image:

```dockerfile
RUN apt-get update && apt-get install -y \
    ros-jazzy-ros-gz \
    libgz-sim8-dev \
    libgz-transport13-dev \
    libgz-msgs10-dev \
    libgz-math7-dev \
    libgz-plugin2-dev \
    libsdformat14-dev \
    libprotobuf-dev \
    protobuf-compiler \
    && rm -rf /var/lib/apt/lists/*
```

## Build

```bash
source /opt/ros/jazzy/setup.bash
cd ~/colcon_ws
colcon build --packages-select pgm_map_creator
source install/setup.bash
```

## Usage

### 1. Run with the launch file

```bash
ros2 launch pgm_map_creator generate_map.launch.py \
    world_name:=my_world.sdf \
    output_path:=/home/user/maps/my_map \
    xmin:=-15 xmax:=15 ymin:=-15 ymax:=15 \
    scan_height:=5 resolution:=0.01
```

### 3. Or run manually

Terminal 1 — Start gz sim with your world:
```bash
export GZ_SIM_SYSTEM_PLUGIN_PATH=$(pwd)/install/lib/pgm_map_creator
gz sim -s -r your_world.sdf
```

Terminal 2 — Publish the map request:
```bash
request_publisher '(-15,15)(15,15)(15,-15)(-15,-15)' 5 0.01 /output/path/map
```

The output will be written to `/output/path/map.pgm`.

## Parameters

| Parameter | Description | Default |
|-----------|-------------|---------|
| `world_file` | Path to the .sdf world file | (required) |
| `output_path` | Full output path without `.pgm` extension | `/tmp/map` |
| `xmin` | Minimum X coordinate | `-15` |
| `xmax` | Maximum X coordinate | `15` |
| `ymin` | Minimum Y coordinate | `-15` |
| `ymax` | Maximum Y coordinate | `15` |
| `scan_height` | Height from which to project collisions | `5` |
| `resolution` | Map resolution in meters/pixel | `0.01` |

## Notes

- This uses bounding-box intersection rather than exact ray casting (which is not
  yet available in Gz Harmonic). For most architectural models (walls, boxes), the
  results are equivalent to the classic Gazebo version.
- The ground plane is excluded automatically since it has zero height at Z=0.
- For mesh geometries, an approximate bounding box is used based on the mesh scale.
