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

### 命令行参数

两个程序都支持可选的网络接口参数：

```bash
./build/bin/lowcmd_publisher [network_interface]
./build/bin/lowcmd_subscriber [network_interface]
```

- **network_interface**: 可选参数，指定使用的网络接口（如 `eth0`, `enp3s0` 等）
- 如果不指定，将使用所有可用网络接口（默认行为）

### 1. 启动订阅者（终端 1）

使用默认网络接口：
```bash
./build/bin/lowcmd_subscriber
```

或指定特定网络接口：
```bash
./build/bin/lowcmd_subscriber eth0
```

订阅者将等待并接收 `rt/lowcmd` 话题的消息，并打印接收到的控制指令。

### 2. 启动发布者（终端 2）

使用默认网络接口：
```bash
./build/bin/lowcmd_publisher
```

或指定特定网络接口（需要与订阅者使用相同的网络接口）：
```bash
./build/bin/lowcmd_publisher eth0
```

发布者将以 100Hz 的频率发布 LowCmd 消息，前 10 个关节使用正弦波作为目标位置示例。

### 3. 查看可用网络接口

```bash
ip addr show
# 或
ifconfig
```

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

## 🔥 局域网通信问题（防火墙配置）

### 问题：局域网无法通信

如果在不同机器间运行程序无法通信，**最常见的原因是防火墙阻止了 DDS 通信**。

### 快速解决方案

**方法 1：使用自动配置脚本（推荐）**

```bash
# 运行防火墙配置脚本，自动开放 DDS 端口
sudo ./setup_firewall.sh
```

**方法 2：手动配置防火墙**

```bash
# Ubuntu/Debian (UFW)
sudo ufw allow 7400:7419/udp

# CentOS/RHEL (firewalld)
sudo firewall-cmd --permanent --add-port=7400-7419/udp
sudo firewall-cmd --reload
```

**方法 3：诊断网络问题**

```bash
# 运行网络诊断脚本
sudo ./check_network.sh
```

### DDS 通信端口

- **UDP 端口范围**：7400-7419
- **默认发现端口**：7410
- **组播地址**：239.255.0.1

详细解决方案请查看：**[局域网通信问题解决方案.md](./局域网通信问题解决方案.md)**

## 注意事项

⚠️ **警告**: 
1. 这是一个**测试程序**，不应该直接用于控制真实机器人
2. 在真实机器人上使用前，请务必：
   - 理解每个参数的含义
   - 设置合理的控制参数
   - 实现正确的 CRC 校验
   - 添加安全保护机制
3. 当前示例中的正弦波控制仅用于演示数据发布，实际应用需要根据机器人运动规划设置合适的控制指令

### ⚠️ 网络接口参数已知问题

**问题**：即使发布者和订阅者指定不同的网络接口，它们在同一台机器上仍然可以通信。

**根本原因**：这是 DDS（数据分发服务）的设计特性：
1. **共享内存传输**：DDS 检测到进程在同一台机器时，会自动使用共享内存通信，完全绕过网络接口
2. **Localhost 优化**：即使禁用共享内存，DDS 也会通过 localhost (127.0.0.1) 通信
3. **性能优化**：这是为了在本地进程间提供最快的通信性能

**结论**：在同一台机器上**无法通过网络接口实现真正的隔离**。要测试网络接口隔离，需要使用不同的机器或容器。

此外，SDK 的 `ChannelFactory::Init()` 函数也可能没有正确将网络接口参数应用到 DDS 配置中。

**临时解决方案**：使用提供的辅助脚本或手动配置环境变量：

#### 方法 1：使用辅助脚本（推荐）

```bash
# 终端 1：启动订阅者
./example/lowcmd_test/run_with_network_interface.sh wwan0 subscriber

# 终端 2：启动发布者
./example/lowcmd_test/run_with_network_interface.sh wwan0 publisher
```

#### 方法 2：手动设置环境变量

```bash
# 绑定到特定网络接口（如 wwan0）
export CYCLONEDDS_URI='<CycloneDDS><Domain><General><Interfaces><NetworkInterface name="wwan0"/></Interfaces></General></Domain></CycloneDDS>'
./build/bin/lowcmd_publisher
./build/bin/lowcmd_subscriber
```

详细信息请参考：[network_interface_issue.md](network_interface_issue.md)

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
