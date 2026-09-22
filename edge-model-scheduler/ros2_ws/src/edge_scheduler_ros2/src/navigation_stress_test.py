#!/usr/bin/env python3
import rclpy
from rclpy.node import Node
from sensor_msgs.msg import Image
from geometry_msgs.msg import Twist
import time

class NavigationStressTestNode(Node):
    def __init__(self):
        super().__init__('navigation_stress_test_node')
        self.image_pub = self.create_publisher(Image, '/camera/image_raw', 10)
        self.cmd_sub = self.create_subscription(Twist, '/cmd_vel', self.cmd_vel_callback, 10)

        self.timer = self.create_timer(1.0 / 10.0, self.publish_frame) # 10 FPS
        self.frame_count = 0
        self.robot_stopped = False
        self.obstacle_active = False
        self.obstacle_trigger_time = 0.0

        # Preallocate valid byte buffer (100x100 RGB)
        self.dummy_data = bytes(100 * 100 * 3)
        self.get_logger().info("Navigation Stress Test Active: 10 FPS stream with periodic contention bursts.")

    def publish_frame(self):
        self.frame_count += 1
        msg = Image()
        msg.header.stamp = self.get_clock().now().to_msg()
        msg.header.frame_id = "camera_link"
        msg.height = 100
        msg.width = 100
        msg.encoding = "bgr8"
        msg.step = 100 * 3
        msg.data = self.dummy_data

        if self.frame_count % 30 == 0:
            self.obstacle_active = True
            self.robot_stopped = False
            self.obstacle_trigger_time = time.time()
            self.get_logger().warn("⚠️ [OBSTACLE DETECTED] Triggering emergency stop deadline (<40ms)...")

        self.image_pub.publish(msg)

    def cmd_vel_callback(self, msg: Twist):
        if self.obstacle_active and msg.linear.x == 0.0 and not self.robot_stopped:
            elapsed_ms = (time.time() - self.obstacle_trigger_time) * 1000.0
            self.robot_stopped = True
            self.obstacle_active = False

            if elapsed_ms <= 40.0:
                self.get_logger().info(f"✅ [SAFE STOP SUCCESS] Robot stopped in {elapsed_ms:.2f} ms (Deadline Met).")
            else:
                self.get_logger().error(f"💥 [COLLISION FAILURE] Robot stopped in {elapsed_ms:.2f} ms (>40ms deadline missed!).")

def main(args=None):
    rclpy.init(args=args)
    node = NavigationStressTestNode()
    rclpy.spin(node)
    node.destroy_node()
    rclpy.shutdown()

if __name__ == '__main__':
    main()
