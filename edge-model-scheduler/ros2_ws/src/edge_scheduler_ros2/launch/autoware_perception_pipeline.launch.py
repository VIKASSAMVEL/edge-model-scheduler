import os
from launch import LaunchDescription
from launch_ros.actions import Node

def generate_launch_description():
    """
    Autoware-Grade Perception Pipeline with EdgeModelScheduler
    Launches:
      1. MultiCameraPerceptionNode (Front Wide, Left Surround, Right Surround)
      2. ObstacleArbiterNode (Failsafe Emergency Brake / Dynamic Collision Arbiter)
    """
    return LaunchDescription([
        # 1. Multi-Camera Surround Perception Pipeline
        Node(
            package='edge_scheduler_ros2',
            executable='multi_camera_perception_node',
            name='multi_camera_perception_node',
            output='screen',
            parameters=[{
                'front_camera_fps': 60,
                'surround_camera_fps': 30,
                'front_deadline_ms': 35.0,
                'enable_zero_copy_shm': True
            }]
        ),
        # 2. Obstacle & Collision Arbiter Node
        Node(
            package='edge_scheduler_ros2',
            executable='obstacle_arbiter_node',
            name='obstacle_arbiter_node',
            output='screen',
            parameters=[{
                'safety_distance_threshold_m': 3.5,
                'emergency_stop_latency_limit_ms': 15.0
            }]
        ),
    ])
