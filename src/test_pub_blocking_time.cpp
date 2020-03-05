#include <chrono>
#include <cstring>
#include <memory>
#include <iostream>
#include <vector>
#include <algorithm>
#include <rclcpp/rclcpp.hpp>
#include <std_msgs/msg/int32.hpp>

class Producer : public rclcpp::Node
{
public:
    Producer(const std::string& topic, const rclcpp::QoS& qos, int rate) : rclcpp::Node("producer")
    {
        pub_ = create_publisher<std_msgs::msg::Int32>(topic, qos);
        using namespace std::chrono_literals;
        timer_ = create_wall_timer(std::chrono::duration<float>(1./rate), [this](){
            msg_.data = count_++;
            auto start_time = std::chrono::steady_clock::now();
            try
            {
                pub_->publish(msg_);
            }
            catch (const std::exception& ex)
            {
                auto dur = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now() - start_time).count();
                std::cout << "publish(): " << ex.what() << std::endl;
                std::cout << "  took " << dur << "ns" << std::endl;
            }
            auto dur = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now() - start_time).count();
            durations_.push_back(dur);
            if (durations_.size() > 500)
            {
                std::sort(durations_.begin(), durations_.end());
                // only report if peaking above 1ms
                if (durations_.back() > (uint32_t)1e6)
                    std::cout << "Pub timer spike (ns): min " << durations_.front() << ", max " << durations_.back() << ", median " << durations_.at(durations_.size()/2) << std::endl;
                durations_.clear();
            }
        });
    }
private:
    int count_ = 0;
    std_msgs::msg::Int32 msg_;
    rclcpp::Publisher<std_msgs::msg::Int32>::SharedPtr pub_;
    rclcpp::TimerBase::SharedPtr timer_;
    std::vector<uint32_t> durations_;
};

int main(int argc, char** argv)
{
    bool is_reliable = true;
    int depth = 1;
    int rate = 100;
    for (int i=1; i<argc; i++)
    {
        if (std::strcmp(argv[i], "--reliable") == 0)
            is_reliable = true;
        if (std::strcmp(argv[i], "--best-effort") == 0)
            is_reliable = false;
        if (std::strcmp(argv[i], "--depth") == 0 && i+1 < argc)
            depth = std::atoi(argv[i+1]);
        if (std::strcmp(argv[i], "--rate") == 0 && i+1 < argc)
            rate = std::atoi(argv[i+1]);
    }

    std::cout << (is_reliable? "reliable" : "best-effort")
        << ", depth=" << depth << std::endl;

    rclcpp::init(argc, argv);
    rclcpp::QoS qos = is_reliable? rclcpp::QoS(depth) : rclcpp::SensorDataQoS().keep_last(depth);
    auto producer = std::make_shared<Producer>("test_topic", qos, rate);
    rclcpp::executors::SingleThreadedExecutor exe;
    exe.add_node(producer);
    exe.spin();
    rclcpp::shutdown();
}
