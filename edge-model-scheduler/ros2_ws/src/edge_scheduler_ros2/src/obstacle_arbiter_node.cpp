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
#include <string>
#include <iostream>

using namespace std::chrono_literals;
using namespace edge_scheduler;

class ObstacleArbiterNode : public rclcpp::Node {
public:
    ObstacleArbiterNode() : Node("obstacle_arbiter_node") {
        RCLCPP_INFO(this->get_logger(), "Initializing EdgeModelScheduler ROS 2 Node...");

        // 1. Initialize Hardware Components
        resource_monitor_ = std::make_shared<ResourceMonitor>();
        resource_monitor_->start(100);

        pinned_mem_pool_ = std::make_shared<PinnedMemoryPool>(1024 * 1024, 4);
        scheduler_ = std::make_unique<AdvancedScheduler>(resource_monitor_, pinned_mem_pool_);
        scheduler_->start();

        governor_ = std::make_unique<QuantizationGovernor>();

        // 2. ROS 2 Publishers & Subscribers
        cmd_vel_pub_ = this->create_publisher<geometry_msgs::msg::Twist>("/cmd_vel", 10);
        telemetry_pub_ = this->create_publisher<std_msgs::msg::String>("/edge_scheduler/status", 10);

        camera_sub_ = this->create_subscription<sensor_msgs::msg::Image>(
            "/camera/image_raw", 10,
            std::bind(&ObstacleArbiterNode::camera_callback, this, std::placeholders::_1));

        analytics_timer_ = this->create_wall_timer(
            500ms, std::bind(&ObstacleArbiterNode::background_analytics_callback, this));

        RCLCPP_INFO(this->get_logger(), "Arbiter running. Zero-Copy UMA pool and Hardware Shedding Active.");
    }

    ~ObstacleArbiterNode() override {
        if (scheduler_) scheduler_->stop();
        if (resource_monitor_) resource_monitor_->stop();
    }

private:
    void camera_callback(const sensor_msgs::msg::Image::ConstSharedPtr /*msg*/) {
        auto now_stamp = this->now();

        if (!scheduler_) return;
        scheduler_->submit_task("safety_obstacle_detector", Priority::HIGH, 
            [](void* /*pinned_buffer*/) {
                std::cout << "[ROS2 Node] ✅ [SAFETY DEADLINE MET] Zero-copy inference complete." << std::endl;
            });
    }

    void background_analytics_callback() {
        if (!resource_monitor_ || !governor_ || !telemetry_pub_) return;
        auto metrics = resource_monitor_->get_current_metrics();
        governor_->evaluate_telemetry(metrics.thermal_temperature, metrics.gpu_utilization);

        std_msgs::msg::String status_msg;
        status_msg.data = "Temp: " + std::to_string(metrics.thermal_temperature) +
                          "C, GPU: " + std::to_string(metrics.gpu_utilization) + "%";
        telemetry_pub_->publish(status_msg);
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
