cmake_minimum_required(VERSION 3.5)

project("imu_publisher")

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_EXTENSIONS OFF)

# Find requirements
find_package(fastcdr REQUIRED)
find_package(fastrtps REQUIRED)

# Include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/types
    ${CMAKE_CURRENT_SOURCE_DIR}/publisher
    ${CMAKE_CURRENT_SOURCE_DIR}/subscriber
)

# Type libraries
add_library(Time_lib types/Time.cxx)
target_link_libraries(Time_lib fastcdr fastrtps)

add_library(Vector3_lib types/Vector3.cxx)
target_link_libraries(Vector3_lib fastcdr fastrtps)

add_library(Quaternion_lib types/Quaternion.cxx)
target_link_libraries(Quaternion_lib fastcdr fastrtps)

add_library(Header_lib types/Header.cxx)
target_link_libraries(Header_lib fastcdr fastrtps)

add_library(Imu_lib types/Imu.cxx)
target_link_libraries(Imu_lib fastcdr fastrtps)

# IMU Publisher executable
message(STATUS "Configuring ImuPublisher...")
add_executable(imu_publisher
    types/ImuPubSubTypes.cxx
    types/HeaderPubSubTypes.cxx
    types/TimePubSubTypes.cxx
    types/Vector3PubSubTypes.cxx
    types/QuaternionPubSubTypes.cxx
    publisher/ImuPublisher.cxx)

target_link_libraries(imu_publisher
    fastcdr
    fastrtps
    Imu_lib
    Header_lib
    Time_lib
    Vector3_lib
    Quaternion_lib)

# IMU Subscriber executable
message(STATUS "Configuring ImuSubscriber...")
add_executable(imu_subscriber
    types/ImuPubSubTypes.cxx
    types/HeaderPubSubTypes.cxx
    types/TimePubSubTypes.cxx
    types/Vector3PubSubTypes.cxx
    types/QuaternionPubSubTypes.cxx
    subscriber/ImuSubscriber.cxx)

target_link_libraries(imu_subscriber
    fastcdr
    fastrtps
    Imu_lib
    Header_lib
    Time_lib
    Vector3_lib
    Quaternion_lib)
