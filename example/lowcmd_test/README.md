# LowCmd 发布订阅测试程序

## 简介

这是一个用于测试 `rt/lowcmd` 话题发布和订阅的示例程序。该话题用于控制 Unitree G1 人形机器人的全身关节电机。

## 消息类型

- **话题名称**: `rt/lowcmd`
- **消息类型**: `unitree_hg::msg::dds_::LowCmd_`

### LowCmd_ 结构体

```cpp
struct LowCmd_ {
  uint8_t mode_pr;                                      // 并联机构（脚踝和腰部）控制模式 (默认 0) 0:PR, 1:AB
  uint8_t mode_machine;                                 // G1 型号：4：23-DOF；5：29-DOF；6:27DOF(29DOF-锁腰)
  std::array<MotorCmd_, 35> motor_cmd;                  // 身体所有电机控制指令
  std::array<uint32_t, 4> reserve;                      // 保留
  uint32_t crc;                                         // 校验和
};
```

### MotorCmd_ 结构体

```cpp
struct MotorCmd_ {
  uint8_t mode;      // 控制模式
  float q;           // 目标位置 (rad)
  float dq;          // 目标速度 (rad/s)
  float tau;         // 前馈力矩 (N·m)
  float kp;          // 位置增益
  float kd;          // 速度增益
  uint32_t reserve;  // 保留
};
```

## 编译

在 SDK 根目录执行：

```bash
./build.sh
```

编译成功后，可执行文件位于 `build/bin/` 目录：
- `lowcmd_publisher` - 发布者
- `lowcmd_subscriber` - 订阅者

## 使用方法

### 1. 启动订阅者（终端 1）

```bash
./build/bin/lowcmd_subscriber
```

订阅者将等待并接收 `rt/lowcmd` 话题的消息，并打印接收到的控制指令。

### 2. 启动发布者（终端 2）

```bash
./build/bin/lowcmd_publisher
```

发布者将以 100Hz 的频率发布 LowCmd 消息，前 10 个关节使用正弦波作为目标位置示例。

## 程序说明

### 发布者 (lowcmd_publisher.cpp)

- 以 100Hz 频率发布 LowCmd 消息
- 设置控制模式：`mode_pr=0` (PR模式)，`mode_machine=5` (29-DOF)
- 前 10 个关节使用位置控制模式，目标位置为正弦波（仅作演示）
- 其他关节设为待机模式

**关键参数：**
- `mode = 10`: 位置控制模式
- `kp = 20.0`: 位置增益
- `kd = 0.5`: 速度增益

### 订阅者 (lowcmd_subscriber.cpp)

- 订阅 `rt/lowcmd` 话题
- 接收到消息后打印详细信息：
  - 控制模式参数
  - 前 10 个电机的控制指令
  - 激活的电机数量统计

## 注意事项

⚠️ **警告**: 
1. 这是一个**测试程序**，不应该直接用于控制真实机器人
2. 在真实机器人上使用前，请务必：
   - 理解每个参数的含义
   - 设置合理的控制参数
   - 实现正确的 CRC 校验
   - 添加安全保护机制
3. 当前示例中的正弦波控制仅用于演示数据发布，实际应用需要根据机器人运动规划设置合适的控制指令

## 扩展建议

1. **添加 CRC 校验**: 实现真实的 CRC 计算函数
2. **参数配置**: 通过配置文件或命令行参数设置控制参数
3. **安全检查**: 添加关节角度、速度、力矩限制
4. **轨迹规划**: 实现平滑的轨迹规划算法
5. **状态反馈**: 结合 `rt/lowstate` 话题实现闭环控制

## 相关文件

- 发布者源码: `lowcmd_publisher.cpp`
- 订阅者源码: `lowcmd_subscriber.cpp`
- CMake 配置: `CMakeLists.txt`
- 消息定义: `include/unitree/idl/hg/LowCmd_.hpp`
- 电机指令定义: `include/unitree/idl/hg/MotorCmd_.hpp`
