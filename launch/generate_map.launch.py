import os
import tempfile
import xml.etree.ElementTree as ET

from ament_index_python.packages import get_package_prefix, get_package_share_directory
from launch import LaunchDescription
from launch.actions import (
    DeclareLaunchArgument,
    ExecuteProcess,
    OpaqueFunction,
    SetEnvironmentVariable,
    TimerAction,
)
from launch.substitutions import LaunchConfiguration

_pkg_lib   = os.path.join(get_package_prefix('pgm_map_creator'), 'lib', 'pgm_map_creator')
_pkg_share = get_package_share_directory('pgm_map_creator')


def _inject_plugin(context, *args, **kwargs):
    world_name  = context.perform_substitution(LaunchConfiguration('world_name'))
    output_path = LaunchConfiguration('output_path')
    xmin        = LaunchConfiguration('xmin')
    xmax        = LaunchConfiguration('xmax')
    ymin        = LaunchConfiguration('ymin')
    ymax        = LaunchConfiguration('ymax')
    scan_height = LaunchConfiguration('scan_height')
    resolution  = LaunchConfiguration('resolution')

    # Parse the world SDF and inject our plugin into the <world> element
    ET.register_namespace('', 'http://www.w3.org/2001/XMLSchema')
    tree = ET.parse(world_name)
    root = tree.getroot()

    world_el = root.find('world')
    if world_el is None:
        raise RuntimeError(f"No <world> element found in {world_name}")

    plugin_el = ET.SubElement(world_el, 'plugin')
    plugin_el.set('name', 'pgm_map_creator::CollisionMapCreator')
    plugin_el.set('filename', 'collision_map_creator')

    tmp = tempfile.NamedTemporaryFile(suffix='.sdf', delete=False, mode='w')
    tree.write(tmp.name, encoding='unicode', xml_declaration=True)
    tmp.close()

    return [
        ExecuteProcess(
            cmd=['gz', 'sim', '-s', '-r', tmp.name],
            output='screen',
        ),
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
    ]


def generate_launch_description():
    return LaunchDescription([
        # ─── Arguments ──────────────────────────────────────────────────
        DeclareLaunchArgument(
            'world_name',
            description='Path to the .sdf world file',
        ),
        DeclareLaunchArgument('output_path', default_value='/tmp/map',
                              description='Full output path without .pgm extension'),
        DeclareLaunchArgument('xmin', default_value='-15'),
        DeclareLaunchArgument('xmax', default_value='15'),
        DeclareLaunchArgument('ymin', default_value='-15'),
        DeclareLaunchArgument('ymax', default_value='15'),
        DeclareLaunchArgument('scan_height', default_value='5'),
        DeclareLaunchArgument('resolution', default_value='0.01'),

        # ─── Tell gz sim where our plugin .so lives ──────────────────────
        SetEnvironmentVariable(name='GZ_SIM_SYSTEM_PLUGIN_PATH', value=_pkg_lib),

        # ─── Inject plugin into world SDF and launch ─────────────────────
        OpaqueFunction(function=_inject_plugin),
    ])
