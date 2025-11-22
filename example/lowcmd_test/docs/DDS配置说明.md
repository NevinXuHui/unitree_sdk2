# CycloneDDS 配置说明

## 环境变量说明

### `RMW_IMPLEMENTATION` （本项目不需要）

```bash
export RMW_IMPLEMENTATION=rmw_cyclonedds_cpp
```

- **用途**：ROS 2 使用的中间件（RMW）实现选择
- **本项目**：❌ **不需要**，因为 `unitree_sdk2` 直接使用 CycloneDDS，不通过 ROS 2
- **仅在使用 ROS 2 时需要**

### `CYCLONEDDS_URI` （本项目需要）

```bash
export CYCLONEDDS_URI='<CycloneDDS>...</CycloneDDS>'
```

- **用途**：配置 CycloneDDS 的网络接口、组播等参数
- **本项目**：✅ **需要**，用于指定网络接口

## 配置方法

### 方法 1：使用自动生成脚本（最推荐）

为任意网络接口生成配置：

```bash
# 生成 eth10 的配置
./generate_dds_config.sh eth10

# 生成 eth0 的配置
./generate_dds_config.sh eth0

# 查看可用的网络接口
./generate_dds_config.sh
```

生成后会创建：
- `cyclonedds_<接口名>.xml` - XML 配置文件
- `run_with_<接口名>.sh` - 运行脚本

使用：

```bash
# 终端 1: 运行订阅者
./run_with_eth10.sh subscriber

# 终端 2: 运行发布者
./run_with_eth10.sh publisher
```

### 方法 2：使用环境变量（内联 XML）

```bash
# 设置 eth10 接口
export CYCLONEDDS_URI='<CycloneDDS><Domain><General><AllowMulticast>spdp</AllowMulticast><Interfaces><NetworkInterface name="eth10" multicast="default"/></Interfaces></General></Domain></CycloneDDS>'

# 运行程序
../../build/bin/lowcmd_publisher
../../build/bin/lowcmd_subscriber
```

**优点**：简单快速  
**缺点**：不方便修改，XML 必须在一行内

### 方法 3：使用 XML 配置文件

创建 `cyclonedds_eth10.xml`：

```xml
<?xml version="1.0" encoding="UTF-8"?>
<CycloneDDS xmlns="https://cdds.io/config">
  <Domain>
    <General>
      <Interfaces>
        <NetworkInterface name="eth10" priority="default" multicast="default"/>
      </Interfaces>
      <AllowMulticast>spdp</AllowMulticast>
    </General>
  </Domain>
</CycloneDDS>
```

使用配置文件：

```bash
export CYCLONEDDS_URI=file://$PWD/cyclonedds_eth10.xml
../../build/bin/lowcmd_publisher
```

**优点**：配置清晰，易于维护  
**缺点**：需要额外的文件

## 配置参数说明

### 网络接口配置

```xml
<NetworkInterface name="eth10" priority="default" multicast="default"/>
```

- **name**: 网络接口名称（如 `eth0`, `eth10`, `wlan0` 等）
- **priority**: 接口优先级（`default` 或数值）
- **multicast**: 组播设置（`default`, `true`, `false`）

### 组播配置

```xml
<AllowMulticast>spdp</AllowMulticast>
```

可选值：
- **`spdp`**: 仅在 SPDP（服务发现）中使用组播（推荐）
- **`true`**: 所有通信都使用组播
- **`false`**: 禁用组播，仅使用单播
- **`default`**: 使用默认设置

## 完整配置示例

### 基础配置（eth10 接口）

```xml
<?xml version="1.0" encoding="UTF-8"?>
<CycloneDDS xmlns="https://cdds.io/config">
  <Domain>
    <General>
      <Interfaces>
        <NetworkInterface name="eth10" priority="default" multicast="default"/>
      </Interfaces>
      <AllowMulticast>spdp</AllowMulticast>
    </General>
  </Domain>
</CycloneDDS>
```

### 带日志的配置（用于调试）

```xml
<?xml version="1.0" encoding="UTF-8"?>
<CycloneDDS xmlns="https://cdds.io/config">
  <Domain>
    <General>
      <Interfaces>
        <NetworkInterface name="eth10" priority="default" multicast="default"/>
      </Interfaces>
      <AllowMulticast>spdp</AllowMulticast>
    </General>
    
    <!-- 启用详细日志 -->
    <Tracing>
      <Verbosity>finest</Verbosity>
      <OutputFile>dds_eth10.log</OutputFile>
    </Tracing>
  </Domain>
</CycloneDDS>
```

### 禁用共享内存（强制使用网络）

```xml
<?xml version="1.0" encoding="UTF-8"?>
<CycloneDDS xmlns="https://cdds.io/config">
  <Domain>
    <General>
      <Interfaces>
        <NetworkInterface name="eth10" priority="default" multicast="default"/>
      </Interfaces>
      <AllowMulticast>spdp</AllowMulticast>
      
      <!-- 禁用共享内存，强制使用网络 -->
      <SharedMemory>
        <Enable>false</Enable>
      </SharedMemory>
    </General>
  </Domain>
</CycloneDDS>
```

### 多网络接口配置

```xml
<?xml version="1.0" encoding="UTF-8"?>
<CycloneDDS xmlns="https://cdds.io/config">
  <Domain>
    <General>
      <Interfaces>
        <!-- 主接口 -->
        <NetworkInterface name="eth10" priority="10" multicast="default"/>
        <!-- 备用接口 -->
        <NetworkInterface name="eth0" priority="5" multicast="default"/>
      </Interfaces>
      <AllowMulticast>spdp</AllowMulticast>
    </General>
  </Domain>
</CycloneDDS>
```

## 验证配置

### 1. 检查网络接口是否存在

```bash
# 查看所有网络接口
ip addr show

# 检查特定接口
ip addr show eth10
```

### 2. 测试 DDS 通信

```bash
# 终端 1: 启动订阅者（带详细输出）
export CYCLONEDDS_URI='<CycloneDDS><Domain><General><Interfaces><NetworkInterface name="eth10"/></Interfaces></General></Domain></CycloneDDS>'
../../build/bin/lowcmd_subscriber

# 终端 2: 启动发布者
export CYCLONEDDS_URI='<CycloneDDS><Domain><General><Interfaces><NetworkInterface name="eth10"/></Interfaces></General></Domain></CycloneDDS>'
../../build/bin/lowcmd_publisher
```

### 3. 监控网络流量

```bash
# 监控 eth10 上的 DDS 流量
sudo tcpdump -i eth10 -n 'udp portrange 7400-7419'

# 查看组播流量
sudo tcpdump -i eth10 -n 'host 239.255.0.1'
```

## 常见问题

### Q1: 如何知道配置是否生效？

**方法 1**: 启用 DDS 日志

```xml
<Tracing>
  <Verbosity>config</Verbosity>
</Tracing>
```

运行程序后，会在控制台输出配置信息。

**方法 2**: 使用网络监控工具

```bash
sudo tcpdump -i eth10 -n 'udp portrange 7400-7419'
```

如果看到数据包，说明配置生效。

### Q2: 为什么设置了 eth10 但仍从其他接口通信？

可能原因：
1. **同一台机器**：DDS 会使用共享内存，绕过网络接口
2. **配置未生效**：检查环境变量是否正确设置
3. **回退到默认**：接口不存在时，DDS 会使用所有接口

解决方法：
- 在两台不同机器上测试
- 使用 tcpdump 确认流量走向
- 启用 DDS 日志检查配置

### Q3: 同一台机器上如何测试网络接口隔离？

❌ **无法实现真正的隔离**，因为 DDS 会使用共享内存或 localhost。

要真正测试，必须使用：
- 两台不同的物理机器
- 不同的虚拟机（网络隔离）
- Docker 容器（网络命名空间隔离）

### Q4: 如何在脚本中使用配置？

```bash
#!/bin/bash
# 方法 1: 内联 XML
export CYCLONEDDS_URI='<CycloneDDS><Domain><General><Interfaces><NetworkInterface name="eth10"/></Interfaces></General></Domain></CycloneDDS>'
./program

# 方法 2: 配置文件
export CYCLONEDDS_URI=file://$PWD/cyclonedds_eth10.xml
./program
```

## 快速参考

### 查看可用网络接口

```bash
ip addr show | grep -E "^[0-9]+:" | awk '{print $2}' | sed 's/:$//'
```

### 生成配置（推荐方式）

```bash
./generate_dds_config.sh eth10
./run_with_eth10.sh subscriber
./run_with_eth10.sh publisher
```

### 环境变量方式（快速测试）

```bash
export CYCLONEDDS_URI='<CycloneDDS><Domain><General><Interfaces><NetworkInterface name="eth10"/></Interfaces></General></Domain></CycloneDDS>'
../../build/bin/lowcmd_publisher
```

### 配置文件方式（生产环境）

```bash
export CYCLONEDDS_URI=file://$PWD/cyclonedds_eth10.xml
../../build/bin/lowcmd_publisher
```

## 参考资料

- [CycloneDDS 官方配置文档](https://github.com/eclipse-cyclonedds/cyclonedds/blob/master/docs/manual/config.rst)
- [CycloneDDS XML Schema](https://raw.githubusercontent.com/eclipse-cyclonedds/cyclonedds/master/etc/cyclonedds.xsd)
- 本地文档：`network_interface_issue.md`
- 本地文档：`局域网通信问题解决方案.md`
