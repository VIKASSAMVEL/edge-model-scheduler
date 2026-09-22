#include "rclcpp/rclcpp.hpp"
#include "sensor_msgs/msg/image.hpp"
#include "geometry_msgs/msg/twist.hpp"
#include "std_msgs/msg/string.hpp"

#include "AdvancedScheduler.hpp"
#include "ResourceMonitor.hpp"
#include "PinnedMemoryPool.hpp"
#include "QuantizationGovernor.hpp"

#include <chrono>
#include <memory>

using namespace std::chrono_literals;
using namespace edge_scheduler;

class ObstacleArbiterNode : public rclcpp::Node {
public:
    ObstacleArbiterNode() : Node("obstacle_arbiter_node") {
        RCLCPP_INFO(this->get_logger(), "Initializing EdgeModelScheduler ROS 2 Node...");

        // 1. Initialize Hardware Components
        resource_monitor_ = std::make_shared<ResourceMonitor>();
        resource_monitor_->start(50); // 50ms polling

        pinned_mem_pool_ = std::make_shared<PinnedMemoryPool>(1920 * 1080 * 3, 4);
        scheduler_ = std::make_unique<AdvancedScheduler>(resource_monitor_, pinned_mem_pool_);
        scheduler_->start();

        governor_ = std::make_unique<QuantizationGovernor>();

        // 2. ROS 2 Publishers & Subscribers
        cmd_vel_pub_ = this->create_publisher<geometry_msgs::msg::Twist>("/cmd_vel", 10);
        telemetry_pub_ = this->create_publisher<std_msgs::msg::String>("/edge_scheduler/status", 10);

        // Subscribe to primary camera stream
        camera_sub_ = this->create_subscription<sensor_msgs::msg::Image>(
            "/camera/image_raw", 10,
            std::bind(&ObstacleArbiterNode::camera_callback, this, std::placeholders::_1));

        // Background analytics simulated stream (Pose/Tracking/Logging)
        analytics_timer_ = this->create_wall_timer(
            100ms, std::bind(&ObstacleArbiterNode::background_analytics_callback, this));

        RCLCPP_INFO(this->get_logger(), "Arbiter running. Zero-Copy UMA pool and Hardware Shedding Active.");
    }

    ~ObstacleArbiterNode() {
        scheduler_->stop();
        resource_monitor_->stop();
    }

private:
    void camera_callback(const sensor_msgs::msg::Image::SharedPtr msg) {
        // High-Priority Safety Task (Obstacle Detection)
        auto start_time = this->now();

        scheduler_->submit_task("safety_obstacle_detector", Priority::HIGH, [this, msg, start_time](void* /*pinned_buffer*/) {
            // Simulated Inference on zero-copy tensor buffer
            auto now = this->now();
            double latency_ms = (now - start_time).seconds() * 1000.0;

            geometry_msgs::msg::Twist twist;
            if (latency_ms < 40.0) {
                // Deadline met: obstacle detected in time, execute safe stop
                twist.linear.x = 0.0;
                twist.angular.z = 0.0;
                RCLCPP_INFO_THROTTLE(this->get_logger(), *this->get_clock(), 1000,
                    "[SAFETY MET] Latency: %.2f ms. Clean emergency stop executed.", latency_ms);
            } else {
                // Deadline violated (what would happen in naive FIFO)
                RCLCPP_ERROR(this->get_logger(),
                    "[CRITICAL MISS] Latency blowout: %.2f ms! Robot emergency stop delayed!", latency_ms);
            }
            cmd_vel_pub_->publish(twist);
        });
    }

    void background_analytics_callback() {
        // Low-Priority Background Task (Logging/Heatmap)
        scheduler_->submit_task("background_logging", Priority::LOW, [this](void* /*buffer*/) {
            auto metrics = resource_monitor_->get_current_metrics();
            governor_->evaluate_telemetry(metrics.thermal_temperature, metrics.gpu_utilization);

            std_msgs::msg::String status_msg;
            status_msg.data = "Temp: " + std::to_string(metrics.thermal_temperature) +
                              "C, GPU: " + std::to_string(metrics.gpu_utilization) +
                              "%, Throttling: " + (metrics.is_throttling ? "YES" : "NO");
            telemetry_pub_->publish(status_msg);
        });
    }

    std::shared_ptr<ResourceMonitor> resource_monitor_;
    std::shared_ptr<PinnedMemoryPool> pinned_mem_pool_;
    std::unique_ptr<AdvancedScheduler> scheduler_;
    std::unique_ptr<QuantizationGovernor> governor_;

    rclcpp::Publisher<geometry_msgs::msg::Twist>::SharedPtr cmd_vel_pub_;
    rclcpp::Publisher<std_msgs::msg::String>::SharedPtr telemetry_pub_;
    rclcpp::Subscription<sensor_msgs::msg::Image>::SharedPtr camera_sub_;
    rclcpp::TimerBase::SharedPtr analytics_timer_;
};

int main(int argc, char** argv) {
    rclcpp::init(argc, argv);
    auto node = std::make_shared<ObstacleArbiterNode>();
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}
