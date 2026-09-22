import os
from launch import LaunchDescription
from launch_ros.actions import Node

def generate_launch_description():
    return LaunchDescription([
        # 1. EdgeModelScheduler Obstacle Arbiter Node (C++)
        Node(
            package='edge_scheduler_ros2',
            executable='obstacle_arbiter_node',
            name='obstacle_arbiter_node',
            output='screen',
        ),
        # 2. Synthetic Navigation Stress Test Generator (Python)
        Node(
            package='edge_scheduler_ros2',
            executable='navigation_stress_test.py',
            name='navigation_stress_test',
            output='screen',
        ),
    ])
