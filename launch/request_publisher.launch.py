import os

from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, ExecuteProcess, TimerAction
from launch.substitutions import LaunchConfiguration, PathJoinSubstitution, FindPackageShare


def generate_launch_description():
    pkg_share = FindPackageShare('pgm_map_creator')

    map_name = LaunchConfiguration('map_name')
    save_folder = LaunchConfiguration('save_folder')
    world_file = LaunchConfiguration('world_file')
    xmin = LaunchConfiguration('xmin')
    xmax = LaunchConfiguration('xmax')
    ymin = LaunchConfiguration('ymin')
    ymax = LaunchConfiguration('ymax')
    scan_height = LaunchConfiguration('scan_height')
    resolution = LaunchConfiguration('resolution')

    return LaunchDescription([
        DeclareLaunchArgument('map_name', default_value='map'),
        DeclareLaunchArgument(
            'save_folder',
            default_value=PathJoinSubstitution([pkg_share, 'maps'])
        ),
        DeclareLaunchArgument(
            'world_file',
            description='Path to the .world/.sdf file to generate map from'
        ),
        DeclareLaunchArgument('xmin', default_value='-15'),
        DeclareLaunchArgument('xmax', default_value='15'),
        DeclareLaunchArgument('ymin', default_value='-15'),
        DeclareLaunchArgument('ymax', default_value='15'),
        DeclareLaunchArgument('scan_height', default_value='5'),
        DeclareLaunchArgument('resolution', default_value='0.01'),

        # Start gz sim with the world file (headless server mode)
        ExecuteProcess(
            cmd=[
                'gz', 'sim', '-s', '-r',
                '--iterations', '1',
                world_file,
            ],
            output='screen',
        ),

        # Delay the request publisher to allow simulation to load
        TimerAction(
            period=5.0,
            actions=[
                ExecuteProcess(
                    cmd=[
                        'request_publisher',
                        ['(', xmin, ',', ymax, ')',
                         '(', xmax, ',', ymax, ')',
                         '(', xmax, ',', ymin, ')',
                         '(', xmin, ',', ymin, ')'],
                        scan_height,
                        resolution,
                        PathJoinSubstitution([save_folder, map_name]),
                    ],
                    output='screen',
                ),
            ],
        ),
    ])
