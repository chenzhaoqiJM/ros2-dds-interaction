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

#include "ImuSubscriber.h"
#include "ImuPubSubTypes.h"
#include "Imu.h"

#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/dds/subscriber/SampleInfo.hpp>
#include <fastdds/dds/subscriber/Subscriber.hpp>
#include <fastdds/dds/subscriber/qos/DataReaderQos.hpp>

#include <csignal>
#include <atomic>
#include <iostream>

using namespace eprosima::fastdds::dds;

// 用于优雅退出的信号处理
static std::atomic<bool> running(true);

void subscriberSignalHandler(int signum)
{
    (void)signum;
    running = false;
}

ImuSubscriber::ImuSubscriber()
    : participant_(nullptr)
    , subscriber_(nullptr)
    , topic_(nullptr)
    , reader_(nullptr)
    , type_(new my_sensor_msgs::msg::ImuPubSubType())
{
}

ImuSubscriber::~ImuSubscriber()
{
    if (reader_ != nullptr)
    {
        subscriber_->delete_datareader(reader_);
    }
    if (topic_ != nullptr)
    {
        participant_->delete_topic(topic_);
    }
    if (subscriber_ != nullptr)
    {
        participant_->delete_subscriber(subscriber_);
    }
    DomainParticipantFactory::get_instance()->delete_participant(participant_);
}

bool ImuSubscriber::init()
{
    // 注册信号处理
    signal(SIGINT, subscriberSignalHandler);
    signal(SIGTERM, subscriberSignalHandler);

    // CREATE THE PARTICIPANT
    DomainParticipantQos pqos;
    pqos.name("Imu_Subscriber");
    participant_ = DomainParticipantFactory::get_instance()->create_participant(0, pqos);
    if (participant_ == nullptr)
    {
        return false;
    }

    // REGISTER THE TYPE
    type_.register_type(participant_);

    // CREATE THE SUBSCRIBER
    subscriber_ = participant_->create_subscriber(SUBSCRIBER_QOS_DEFAULT, nullptr);
    if (subscriber_ == nullptr)
    {
        return false;
    }

    // CREATE THE TOPIC (与 Publisher 使用相同的 topic 名称)
    topic_ = participant_->create_topic(
        "rt/imu",
        type_.get_type_name(),
        TOPIC_QOS_DEFAULT);
    if (topic_ == nullptr)
    {
        return false;
    }

    // CREATE THE READER
    DataReaderQos rqos = DATAREADER_QOS_DEFAULT;
    rqos.reliability().kind = RELIABLE_RELIABILITY_QOS;
    reader_ = subscriber_->create_datareader(topic_, rqos, &listener_);
    if (reader_ == nullptr)
    {
        return false;
    }

    std::cout << "IMU Subscriber initialized. Topic: rt/imu" << std::endl;
    return true;
}

void ImuSubscriber::SubListener::on_subscription_matched(
        DataReader*,
        const SubscriptionMatchedStatus& info)
{
    if (info.current_count_change == 1)
    {
        matched = info.total_count;
        std::cout << "Subscriber matched. Total publishers: " << matched << std::endl;
    }
    else if (info.current_count_change == -1)
    {
        matched = info.total_count;
        std::cout << "Subscriber unmatched. Total publishers: " << matched << std::endl;
    }
}

void ImuSubscriber::SubListener::on_data_available(
        DataReader* reader)
{
    my_sensor_msgs::msg::Imu imu_msg;
    SampleInfo info;

    if (reader->take_next_sample(&imu_msg, &info) == ReturnCode_t::RETCODE_OK)
    {
        if (info.valid_data)
        {
            ++samples;

            // 每 100 条消息打印一次详细信息
            if (samples % 100 == 0)
            {
                std::cout << "[" << samples << "] Received IMU data:" << std::endl;
                std::cout << "  Timestamp: " << imu_msg.header().stamp().sec() << "."
                          << imu_msg.header().stamp().nanosec() << std::endl;
                std::cout << "  Frame ID: " << imu_msg.header().frame_id() << std::endl;
                std::cout << "  Orientation: ("
                          << imu_msg.orientation().x() << ", "
                          << imu_msg.orientation().y() << ", "
                          << imu_msg.orientation().z() << ", "
                          << imu_msg.orientation().w() << ")" << std::endl;
                std::cout << "  Angular velocity: ("
                          << imu_msg.angular_velocity().x() << ", "
                          << imu_msg.angular_velocity().y() << ", "
                          << imu_msg.angular_velocity().z() << ")" << std::endl;
                std::cout << "  Linear acceleration: ("
                          << imu_msg.linear_acceleration().x() << ", "
                          << imu_msg.linear_acceleration().y() << ", "
                          << imu_msg.linear_acceleration().z() << ")" << std::endl;
            }
        }
    }
}

void ImuSubscriber::run()
{
    std::cout << "Subscribing to IMU data. Press Ctrl+C to stop." << std::endl;

    while (running)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    std::cout << "\nStopped. Total messages received: " << listener_.samples << std::endl;
}

int main(int argc, char** argv)
{
    (void)argc;
    (void)argv;

    std::cout << "Starting IMU Subscriber..." << std::endl;

    ImuSubscriber subscriber;
    if (subscriber.init())
    {
        subscriber.run();
    }
    else
    {
        std::cerr << "Failed to initialize IMU Subscriber!" << std::endl;
        return 1;
    }

    return 0;
}
