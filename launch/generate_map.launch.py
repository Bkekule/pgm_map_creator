import os

from ament_index_python.packages import get_package_prefix, get_package_share_directory
from launch import LaunchDescription
from launch.actions import (
    DeclareLaunchArgument,
    ExecuteProcess,
    SetEnvironmentVariable,
    TimerAction,
)
from launch.substitutions import LaunchConfiguration

_pkg_lib  = os.path.join(get_package_prefix('pgm_map_creator'), 'lib', 'pgm_map_creator')
_pkg_share = get_package_share_directory('pgm_map_creator')


def generate_launch_description():
    world_name  = LaunchConfiguration('world_name')
    output_path = LaunchConfiguration('output_path')
    xmin        = LaunchConfiguration('xmin')
    xmax        = LaunchConfiguration('xmax')
    ymin        = LaunchConfiguration('ymin')
    ymax        = LaunchConfiguration('ymax')
    scan_height = LaunchConfiguration('scan_height')
    resolution  = LaunchConfiguration('resolution')

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
        SetEnvironmentVariable(name='GZ_SIM_SYSTEM_PLUGIN_PATH', value=_pkg_lib),

        # ─── Start gz sim headless ──────────────────────────────────────
        ExecuteProcess(
            cmd=['gz', 'sim', '-s', '-r', world_name],
            additional_env={
                'GZ_SIM_SERVER_CONFIG_PATH': os.path.join(_pkg_share, 'config', 'server.config'),
            },
            output='screen',
        ),

        # ─── Publish map request after sim loads ────────────────────────
        TimerAction(
            period=5.0,
            actions=[
                ExecuteProcess(
                    cmd=[
                        os.path.join(_pkg_lib, 'request_publisher'),
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
