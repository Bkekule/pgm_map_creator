import os

from launch import LaunchDescription
from launch.actions import (
    DeclareLaunchArgument,
    ExecuteProcess,
    SetEnvironmentVariable,
    TimerAction,
)
from launch.substitutions import (
    LaunchConfiguration,
    PathJoinSubstitution,  # used for plugin/binary paths
)
from launch_ros.substitutions import FindPackageShare


def generate_launch_description():
    pkg_share = FindPackageShare('pgm_map_creator')

    world_name = LaunchConfiguration('world_name')
    output_path = LaunchConfiguration('output_path')
    xmin = LaunchConfiguration('xmin')
    xmax = LaunchConfiguration('xmax')
    ymin = LaunchConfiguration('ymin')
    ymax = LaunchConfiguration('ymax')
    scan_height = LaunchConfiguration('scan_height')
    resolution = LaunchConfiguration('resolution')

    return LaunchDescription([
        # ─── Arguments ──────────────────────────────────────────────────
        DeclareLaunchArgument(
            'world_name',
            description=(
                'Name of the .sdf world file. Discovered via '
                'GZ_SIM_RESOURCE_PATH, same as running gz sim <name>.sdf'
            ),
        ),
        DeclareLaunchArgument('output_path', default_value='/tmp/map',
                              description='Full output path without .pgm extension'),
        DeclareLaunchArgument('xmin', default_value='-15'),
        DeclareLaunchArgument('xmax', default_value='15'),
        DeclareLaunchArgument('ymin', default_value='-15'),
        DeclareLaunchArgument('ymax', default_value='15'),
        DeclareLaunchArgument('scan_height', default_value='5'),
        DeclareLaunchArgument('resolution', default_value='0.01'),

        # ─── Environment: tell gz sim where our plugin .so lives ────────
        SetEnvironmentVariable(
            name='GZ_SIM_SYSTEM_PLUGIN_PATH',
            value=PathJoinSubstitution([
                pkg_share, '../..', 'lib', 'pgm_map_creator'
            ]),
        ),

        # ─── Start gz sim headless ──────────────────────────────────────
        # GZ_SIM_SERVER_CONFIG_PATH loads our plugin into the world
        # automatically without modifying the .sdf file.
        # world_name is discovered via GZ_SIM_RESOURCE_PATH (standard gz behavior).
        ExecuteProcess(
            cmd=['gz', 'sim', '-s', '-r', '--iterations', '100', world_name],
            additional_env={
                'GZ_SIM_SERVER_CONFIG_PATH': PathJoinSubstitution(
                    [pkg_share, 'config', 'server.config']
                ),
            },
            output='screen',
        ),

        # ─── Publish map request after sim loads ────────────────────────
        TimerAction(
            period=5.0,
            actions=[
                ExecuteProcess(
                    cmd=[
                        PathJoinSubstitution([
                            pkg_share, '../..', 'lib', 'pgm_map_creator',
                            'request_publisher'
                        ]),
                        ['(', xmin, ',', ymax, ')',
                         '(', xmax, ',', ymax, ')',
                         '(', xmax, ',', ymin, ')',
                         '(', xmin, ',', ymin, ')'],
                        scan_height,
                        resolution,
                        output_path,
                    ],
                    output='screen',
                ),
            ],
        ),
    ])
