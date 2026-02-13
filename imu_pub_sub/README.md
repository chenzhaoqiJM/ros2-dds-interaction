# ROS2 DDS Interaction

基于 Fast DDS 实现的 IMU 数据发布/订阅示例，可与 ROS2 节点进行互操作通信。

## 项目简介

本项目演示了如何使用 eProsima Fast DDS 直接实现 DDS 通信，并与 ROS2 生态系统进行数据交互。通过定义与 ROS2 `sensor_msgs/msg/Imu` 兼容的消息类型，实现了原生 DDS 应用与 ROS2 节点之间的无缝通信。

## 功能特性

- 使用 Fast DDS 实现 IMU 数据的发布与订阅
- 与 ROS2 `sensor_msgs/msg/Imu` 消息类型兼容
- 100Hz 数据发布频率
- 支持优雅退出（Ctrl+C）
- Topic 名称：`rt/imu`

## 依赖项

- CMake >= 3.5
- C++11 编译器
- [eProsima Fast DDS](https://github.com/eProsima/Fast-DDS)
- [eProsima Fast CDR](https://github.com/eProsima/Fast-CDR)

### 安装 Fast DDS

```bash
# Ubuntu
sudo apt install ros-<distro>-fastrtps ros-<distro>-fastcdr

# 或从源码编译
# 参考: https://fast-dds.docs.eprosima.com/en/latest/installation/binaries/binaries_linux.html
```

## 编译

```bash
source /opt/ros/humble/setup.bash
cd imu_pub_sub
mkdir build && cd build
cmake ..
make
```

## 运行

### 启动发布者

```bash
./imu_publisher
```

### 启动订阅者

```bash
./imu_subscriber
```

### 与 ROS2 交互

在 ROS2 环境中，可以使用以下命令查看 IMU 数据：

```bash
ros2 topic echo /imu
```

或使用 ROS2 发布 IMU 数据供本项目的订阅者接收。

## 项目结构

```
imu_pub_sub/
├── CMakeLists.txt          # CMake 构建配置
├── publisher/
│   ├── ImuPublisher.cxx    # IMU 发布者实现
│   └── ImuPublisher.h
├── subscriber/
│   ├── ImuSubscriber.cxx   # IMU 订阅者实现
│   └── ImuSubscriber.h
└── types/                  # IDL 生成的消息类型
    ├── Header.*            # std_msgs/Header
    ├── Imu.*               # sensor_msgs/Imu
    ├── Quaternion.*        # geometry_msgs/Quaternion
    ├── Time.*              # builtin_interfaces/Time
    └── Vector3.*           # geometry_msgs/Vector3
```

## 消息类型

IMU 消息包含以下字段：

| 字段 | 类型 | 描述 |
|------|------|------|
| header | Header | 时间戳和坐标系 |
| orientation | Quaternion | 姿态四元数 |
| orientation_covariance | double[9] | 姿态协方差 |
| angular_velocity | Vector3 | 角速度 |
| angular_velocity_covariance | double[9] | 角速度协方差 |
| linear_acceleration | Vector3 | 线性加速度 |
| linear_acceleration_covariance | double[9] | 线性加速度协方差 |

## 许可证

Apache License 2.0
