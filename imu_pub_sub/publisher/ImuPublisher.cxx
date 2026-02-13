// Copyright 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "ImuPublisher.h"
#include "ImuPubSubTypes.h"

#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/publisher/qos/PublisherQos.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/publisher/qos/DataWriterQos.hpp>

#include <thread>
#include <chrono>
#include <cmath>
#include <csignal>
#include <atomic>

using namespace eprosima::fastdds::dds;

// 用于优雅退出的信号处理
static std::atomic<bool> running(true);

void signalHandler(int signum)
{
    (void)signum;
    running = false;
}

ImuPublisher::ImuPublisher()
    : participant_(nullptr)
    , publisher_(nullptr)
    , topic_(nullptr)
    , writer_(nullptr)
    , type_(new my_sensor_msgs::msg::ImuPubSubType())
{
}

ImuPublisher::~ImuPublisher()
{
    if (writer_ != nullptr)
    {
        publisher_->delete_datawriter(writer_);
    }
    if (publisher_ != nullptr)
    {
        participant_->delete_publisher(publisher_);
    }
    if (topic_ != nullptr)
    {
        participant_->delete_topic(topic_);
    }
    DomainParticipantFactory::get_instance()->delete_participant(participant_);
}

bool ImuPublisher::init()
{
    // 注册信号处理
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);

    // CREATE THE PARTICIPANT
    DomainParticipantQos pqos;
    pqos.name("Imu_Publisher");
    participant_ = DomainParticipantFactory::get_instance()->create_participant(0, pqos);
    if (participant_ == nullptr)
    {
        return false;
    }

    // REGISTER THE TYPE
    type_.register_type(participant_);

    // CREATE THE PUBLISHER
    publisher_ = participant_->create_publisher(PUBLISHER_QOS_DEFAULT, nullptr);
    if (publisher_ == nullptr)
    {
        return false;
    }

    // CREATE THE TOPIC
    topic_ = participant_->create_topic(
        "rt/imu",
        type_.get_type_name(),
        TOPIC_QOS_DEFAULT);
    if (topic_ == nullptr)
    {
        return false;
    }

    // CREATE THE WRITER
    writer_ = publisher_->create_datawriter(topic_, DATAWRITER_QOS_DEFAULT, &listener_);
    if (writer_ == nullptr)
    {
        return false;
    }

    std::cout << "IMU Publisher initialized. Topic: rt/imu" << std::endl;
    return true;
}

void ImuPublisher::PubListener::on_publication_matched(
        eprosima::fastdds::dds::DataWriter*,
        const eprosima::fastdds::dds::PublicationMatchedStatus& info)
{
    if (info.current_count_change == 1)
    {
        matched = info.total_count;
        std::cout << "DataWriter matched. Total subscribers: " << matched << std::endl;
    }
    else if (info.current_count_change == -1)
    {
        matched = info.total_count;
        std::cout << "DataWriter unmatched. Total subscribers: " << matched << std::endl;
    }
}

void ImuPublisher::run()
{
    std::cout << "Publishing IMU data at 100Hz. Press Ctrl+C to stop." << std::endl;

    my_sensor_msgs::msg::Imu imu_msg;
    uint32_t msg_count = 0;

    // 100Hz = 10ms 周期
    const auto period = std::chrono::milliseconds(10);
    auto next_time = std::chrono::steady_clock::now();

    while (running)
    {
        // 获取当前时间戳
        auto now = std::chrono::system_clock::now();
        auto duration = now.time_since_epoch();
        auto seconds = std::chrono::duration_cast<std::chrono::seconds>(duration);
        auto nanoseconds = std::chrono::duration_cast<std::chrono::nanoseconds>(duration - seconds);

        // 设置 Header
        imu_msg.header().stamp().sec(static_cast<int32_t>(seconds.count()));
        imu_msg.header().stamp().nanosec(static_cast<uint32_t>(nanoseconds.count()));
        imu_msg.header().frame_id("imu_link");

        // 模拟 IMU 数据 (使用正弦波模拟运动)
        double t = msg_count * 0.01; // 时间 (秒)

        // 模拟姿态四元数 (绕 Z 轴缓慢旋转)
        double angle = 0.1 * sin(0.5 * t);
        imu_msg.orientation().x(0.0);
        imu_msg.orientation().y(0.0);
        imu_msg.orientation().z(sin(angle / 2.0));
        imu_msg.orientation().w(cos(angle / 2.0));

        // 模拟角速度 (rad/s)
        imu_msg.angular_velocity().x(0.01 * sin(2.0 * t));
        imu_msg.angular_velocity().y(0.01 * cos(2.0 * t));
        imu_msg.angular_velocity().z(0.05 * cos(0.5 * t));

        // 模拟线性加速度 (m/s^2), 包含重力
        imu_msg.linear_acceleration().x(0.1 * sin(1.0 * t));
        imu_msg.linear_acceleration().y(0.1 * cos(1.0 * t));
        imu_msg.linear_acceleration().z(9.81 + 0.05 * sin(3.0 * t));

        // 发布消息
        writer_->write(&imu_msg);
        msg_count++;

        // 每秒打印一次状态
        if (msg_count % 100 == 0)
        {
            std::cout << "[" << msg_count << "] Published IMU data. "
                      << "Orientation: (" << imu_msg.orientation().x() << ", "
                      << imu_msg.orientation().y() << ", "
                      << imu_msg.orientation().z() << ", "
                      << imu_msg.orientation().w() << ")" << std::endl;
        }

        // 精确控制 100Hz 频率
        next_time += period;
        std::this_thread::sleep_until(next_time);
    }

    std::cout << "\nStopped. Total messages sent: " << msg_count << std::endl;
}

int main(int argc, char** argv)
{
    (void)argc;
    (void)argv;

    std::cout << "Starting IMU Publisher..." << std::endl;

    ImuPublisher publisher;
    if (publisher.init())
    {
        publisher.run();
    }
    else
    {
        std::cerr << "Failed to initialize IMU Publisher!" << std::endl;
        return 1;
    }

    return 0;
}
