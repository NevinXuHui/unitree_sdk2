# 辅助脚本和配置文件

本目录包含高级配置工具和配置示例文件。

## 📂 目录内容

### 脚本工具

| 脚本 | 功能 | 使用场景 |
|------|------|----------|
| `check_network.sh` | 基础网络检查 | 检查网络配置和 DDS 端口 |
| `generate_dds_config.sh` | 配置生成工具 | 为任意网络接口生成配置 |
| `test_config.sh` | 配置测试工具 | 验证 DDS 配置是否正确 |

### 配置示例

| 文件 | 用途 | 适用场景 |
|------|------|----------|
| `cyclonedds_eth10.xml` | eth10 网络接口配置 | 使用 eth10 接口 |
| `cyclonedds_tailscale_unicast.xml` | Tailscale 单播配置 | 使用 Tailscale VPN |

## 🛠️ 脚本使用

### check_network.sh

**功能：** 检查基础网络配置

**使用：**
```bash
./check_network.sh
```

**检查内容：**
- 网络接口状态
- 防火墙配置
- DDS 端口监听
- 组播支持

---

### generate_dds_config.sh

**功能：** 为指定网络接口生成完整的 DDS 配置

**使用：**
```bash
./generate_dds_config.sh <网络接口名>

# 示例
./generate_dds_config.sh wlp2s0
```

**生成内容：**
- XML 配置文件
- 运行脚本
- 测试脚本

**输出位置：**
- `cyclonedds_<接口名>.xml`
- `run_with_<接口名>.sh`

---

### test_config.sh

**功能：** 测试 DDS 配置是否正确

**使用：**
```bash
./test_config.sh <网络接口名>

# 示例
./test_config.sh wlp2s0
```

**检查内容：**
- 网络接口是否存在
- XML 配置语法
- 必需配置项
- 参数格式

---

## 📄 配置文件说明

### cyclonedds_eth10.xml

**用途：** eth10 网络接口的 DDS 配置示例

**关键配置：**
```xml
<NetworkInterface name="eth10" priority="default" multicast="default"/>
<AllowMulticast>spdp</AllowMulticast>
```

**使用方法：**
```bash
export CYCLONEDDS_URI=file://$PWD/cyclonedds_eth10.xml
../../build/bin/lowcmd_publisher
```

---

### cyclonedds_tailscale_unicast.xml

**用途：** Tailscale VPN 的单播配置（禁用组播）

**关键配置：**
```xml
<NetworkInterface name="tailscale0" priority="default" multicast="false"/>
<AllowMulticast>false</AllowMulticast>
```

**特点：**
- 完全禁用组播（Tailscale 不支持）
- 使用纯单播模式
- 可选：手动配置对等节点

**使用方法：**
```bash
export CYCLONEDDS_URI=file://$PWD/cyclonedds_tailscale_unicast.xml
../../build/bin/lowcmd_publisher
```

---

## 💡 使用建议

### 什么时候使用这些工具？

**使用 check_network.sh：**
- 快速检查网络配置
- 验证 DDS 端口状态
- 确认防火墙规则

**使用 generate_dds_config.sh：**
- 为新的网络接口生成配置
- 创建自定义配置脚本
- 学习配置文件格式

**使用 test_config.sh：**
- 验证手动编写的配置
- 调试配置问题
- 检查配置语法

**使用配置文件：**
- 需要固定配置时
- 脚本配置不满足需求时
- 需要高级配置选项时

---

## 🔄 与核心脚本的关系

### 核心脚本（上级目录）

大多数用户应该使用上级目录的核心脚本：
- `run_test.sh` - 本地测试
- `run_with_network_interface.sh` - 跨机器通信
- `setup_firewall.sh` - 防火墙配置
- `diagnose_lan_communication.sh` - 诊断工具

### 本目录工具

本目录的工具适合：
- 需要自定义配置的用户
- 想深入了解 DDS 配置的用户
- 需要为新接口生成配置的用户

---

## 📖 相关文档

- [DDS配置说明](../docs/DDS配置说明.md) - 详细配置指南
- [环境变量标准格式](../docs/环境变量标准格式.md) - 配置格式说明
- [主 README](../README.md) - 快速开始指南

---

## 💡 提示

1. **大多数用户不需要直接使用这些工具** - 核心脚本已经足够
2. **配置示例可以作为参考** - 学习配置格式
3. **generate_dds_config.sh 很有用** - 为新接口快速生成配置
4. **test_config.sh 帮助调试** - 验证手动配置

---

## 🔗 返回

- [返回主目录](../)
- [查看文档](../docs/)
- [查看目录结构](../STRUCTURE.md)
