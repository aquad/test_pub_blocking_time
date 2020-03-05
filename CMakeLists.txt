cmake_minimum_required(VERSION 3.5)
project(test_pub_blocking_time)

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(std_msgs REQUIRED)

add_executable(test_pub_blocking_time src/test_pub_blocking_time.cpp)
set_target_properties(test_pub_blocking_time PROPERTIES CXX_STANDARD 14)
ament_target_dependencies(test_pub_blocking_time rclcpp std_msgs)

install(TARGETS test_pub_blocking_time
        EXPORT test_pub_blocking_time
        DESTINATION lib/${PROJECT_NAME})

ament_package()
