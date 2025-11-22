# 网络接口配置问题说明

## 问题描述

当前 `lowcmd_test` 程序虽然支持通过命令行参数指定网络接口，但**网络接口隔离并未真正生效**。

### 测试结果

```bash
# 发布者使用 wwan0
./build/bin/lowcmd_publisher wwan0

# 订阅者使用 tailscale0
./build/bin/lowcmd_subscriber tailscale0
```

**预期行为**：发布者和订阅者使用不同的网络接口，应该无法通信。

**实际行为**：发布者和订阅者仍然可以正常通信，说明网络接口限制未生效。

## 根本原因

### ⚠️ 重要：DDS 本地通信机制

**即使配置了网络接口，在同一台机器上运行的发布者和订阅者也能通信。**这是因为：

1. **共享内存传输（Shared Memory Transport）**：CycloneDDS 在检测到发布者和订阅者在同一台机器上时，会自动使用共享内存进行数据传输，完全绕过网络栈。

2. **Localhost Bypass**：即使禁用了共享内存，DDS 也会使用 `localhost (127.0.0.1)` 进行本地通信，这同样绕过了网络接口限制。

3. **性能优化设计**：这是 DDS 的设计特性，目的是在同一台机器上提供最快的通信性能。

### 其他可能的问题

除了上述根本性限制，还可能存在：

1. **SDK 实现问题**：`libunitree_sdk2.a` 可能没有正确将 `networkInterface` 参数转换为 CycloneDDS 配置
2. **DDS 配置回退**：当指定的网络接口无效时，CycloneDDS 可能自动回退到使用所有可用接口
3. **组播模式**：CycloneDDS 默认使用组播（multicast），可能跨越多个网络接口进行通信

## 临时解决方案

### 方案 1：使用环境变量配置 CycloneDDS

通过 `CYCLONEDDS_URI` 环境变量直接配置 DDS，绕过 SDK 的网络接口参数：

```bash
# 发布者绑定到 wwan0
export CYCLONEDDS_URI='<CycloneDDS><Domain><General><Interfaces><NetworkInterface name="wwan0" priority="default" multicast="true"/></Interfaces></General></Domain></CycloneDDS>'
./build/bin/lowcmd_publisher

# 订阅者绑定到 wwan0
export CYCLONEDDS_URI='<CycloneDDS><Domain><General><Interfaces><NetworkInterface name="wwan0" priority="default" multicast="true"/></Interfaces></General></Domain></CycloneDDS>'
./build/bin/lowcmd_subscriber
```

### 方案 2：使用配置文件

创建 CycloneDDS 配置文件 `cyclonedds_wwan0.xml`：

```xml
<?xml version="1.0" encoding="UTF-8"?>
<CycloneDDS xmlns="https://cdds.io/config" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xsi:schemaLocation="https://cdds.io/config https://raw.githubusercontent.com/eclipse-cyclonedds/cyclonedds/master/etc/cyclonedds.xsd">
  <Domain>
    <General>
      <Interfaces>
        <NetworkInterface name="wwan0" priority="default" multicast="true"/>
      </Interfaces>
    </General>
  </Domain>
</CycloneDDS>
```

使用配置文件：

```bash
export CYCLONEDDS_URI=file://$PWD/cyclonedds_wwan0.xml
./build/bin/lowcmd_publisher
./build/bin/lowcmd_subscriber
```

### 方案 3：禁用组播，使用单播

如果要确保网络隔离，可以禁用组播：

```xml
<?xml version="1.0" encoding="UTF-8"?>
<CycloneDDS xmlns="https://cdds.io/config">
  <Domain>
    <General>
      <Interfaces>
        <NetworkInterface name="wwan0"/>
      </Interfaces>
      <AllowMulticast>false</AllowMulticast>
    </General>
  </Domain>
</CycloneDDS>
```

## 验证网络接口隔离

### ⚠️ 重要限制

**在同一台机器上无法真正验证网络接口隔离**，因为 DDS 会使用共享内存或 localhost 绕过网络接口。

要真正测试网络接口隔离，需要：

### 方法 1：使用两台不同的机器

```bash
# 机器 A (IP: 10.69.145.153，通过 wwan0)
export CYCLONEDDS_URI='<CycloneDDS><Domain><General><Interfaces><NetworkInterface name="wwan0"/></Interfaces></General></Domain></CycloneDDS>'
./build/bin/lowcmd_publisher

# 机器 B (必须能通过 wwan0 网络访问到机器 A)
export CYCLONEDDS_URI='<CycloneDDS><Domain><General><Interfaces><NetworkInterface name="eth0"/></Interfaces></General></Domain></CycloneDDS>'
./build/bin/lowcmd_subscriber
```

### 方法 2：使用 Docker 容器（网络命名空间隔离）

```bash
# 创建两个使用不同网络接口的容器
docker run --network=host -e CYCLONEDDS_URI='...' ...
```

### 方法 3：禁用共享内存传输（仅用于测试）

```xml
<?xml version="1.0" encoding="UTF-8"?>
<CycloneDDS xmlns="https://cdds.io/config">
  <Domain>
    <General>
      <Interfaces>
        <NetworkInterface name="wwan0"/>
      </Interfaces>
      <!-- 禁用共享内存，强制使用网络 -->
      <SharedMemory>
        <Enable>false</Enable>
      </SharedMemory>
      <!-- 禁用 localhost，强制使用真实网络接口 -->
      <DontRoute>true</DontRoute>
    </General>
  </Domain>
</CycloneDDS>
```

**注意**：即使这样配置，DDS 仍然可能通过 localhost 通信，因为这是操作系统级别的路由行为。

### 本地测试（仅供参考，无法真正隔离）

```bash
# 终端 1：发布者使用 wwan0
export CYCLONEDDS_URI='<CycloneDDS><Domain><General><Interfaces><NetworkInterface name="wwan0"/></Interfaces></General></Domain></CycloneDDS>'
./build/bin/lowcmd_publisher

# 终端 2：订阅者使用 tailscale0（不同网络接口）
export CYCLONEDDS_URI='<CycloneDDS><Domain><General><Interfaces><NetworkInterface name="tailscale0"/></Interfaces></General></Domain></CycloneDDS>'
./build/bin/lowcmd_subscriber
```

**实际结果**：由于共享内存/localhost 机制，订阅者仍然能收到消息。

## 后续建议

1. **联系宇树官方**：报告 `ChannelFactory::Init()` 的 `networkInterface` 参数未生效的问题
2. **使用环境变量**：在问题修复前，使用 `CYCLONEDDS_URI` 环境变量作为临时解决方案
3. **更新文档**：在 README 中说明当前网络接口参数的限制

## 参考资料

- [CycloneDDS 配置文档](https://github.com/eclipse-cyclonedds/cyclonedds/blob/master/docs/manual/config.rst)
- CycloneDDS 网络接口配置路径：`/CycloneDDS/Domain/General/Interfaces/NetworkInterface`
