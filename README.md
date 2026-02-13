
# ROS2-DDS-Interaction

这是一个 DDS 与 ROS2 互操作的示例仓库，直接使用 ROS2 提供的 Fast DDS 库，支持纯 DDS 收发以及 DDS 与 ROS2 互通。

## 功能特性

- 纯 DDS 发布/订阅通信
- DDS 与 ROS2 节点互操作
- 多种消息类型示例

## 示例列表

| 示例名称 | 消息类型 | 描述 |
|---------|---------|------|
| [imu_pub_sub](./imu_pub_sub) | sensor_msgs/Imu | IMU 传感器数据发布订阅示例 |

## 环境要求

- ROS2 (Humble 或更高版本)
- Fast DDS
- CMake 3.16+
- C++17

## 快速开始

### 编译

```bash
cd <示例目录>
mkdir build && cd build
cmake ..
make
```

### 运行

请参考各示例目录下的 README.md 获取详细运行说明。

## 目录结构

```
ros2-dds-interaction/
├── README.md
├── LICENSE
└── imu_pub_sub/          # IMU 消息示例
    ├── publisher/        # 发布者实现
    ├── subscriber/       # 订阅者实现
    └── types/            # IDL 生成的类型文件
```

## 添加新示例

1. 创建新的示例目录
2. 使用 `ros2 interface show` 查看消息定义
3. 使用 Fast DDS Gen 生成类型文件
4. 实现发布者和订阅者
5. 更新本 README 的示例列表

## 许可证

本项目采用 MIT 许可证，详见 [LICENSE](./LICENSE) 文件。