# LowCmd 测试程序使用指南

> 💡 **提示：** 首次使用请先查看 [目录结构说明](STRUCTURE.md)

## 📖 目录

- [快速开始](#快速开始)
- [跨机器通信](#跨机器通信)
- [脚本说明](#脚本说明)
- [故障排查](#故障排查)
- [详细文档](#详细文档)

## 📂 相关文档

- **[STRUCTURE.md](STRUCTURE.md)** - 目录结构和文件说明
- **[docs/INDEX.md](docs/INDEX.md)** - 完整文档索引

---

## 🚀 快速开始

### 1. 编译程序

```bash
cd /mine/Code/unitree/unitree_sdk2
./build.sh
```

### 2. 同一台机器测试

```bash
cd example/lowcmd_test

# 方法 1：使用测试脚本（推荐）
./run_test.sh

# 方法 2：手动运行
终端1: ../../build/bin/lowcmd_subscriber
终端2: ../../build/bin/lowcmd_publisher
```

---

## 🌐 跨机器通信

### 前提条件

- ✅ 两台机器在同一个局域网
- ✅ 能互相 ping 通
- ✅ 两台都配置防火墙

### 配置步骤

#### 步骤 1：两台机器都配置防火墙

```bash
# 在机器 A 和机器 B 上都执行
cd /mine/Code/unitree/unitree_sdk2/example/lowcmd_test
sudo ./setup_firewall.sh
```

#### 步骤 2：启动订阅者（机器 A）

```bash
./run_with_network_interface.sh wlp2s0 subscriber
```

#### 步骤 3：启动发布者（机器 B）

```bash
./run_with_network_interface.sh wlp2s0 publisher
```

### 网络接口选择

查看可用网络接口：

```bash
ip addr show
```

常见接口：
- `eth0` - 有线网卡
- `wlp2s0` - 无线网卡
- `enp3s0` - 以太网
- `tailscale0` - Tailscale VPN

---

## 📋 脚本说明

### 核心脚本

| 脚本 | 用途 | 使用场景 |
|------|------|----------|
| `run_test.sh` | 交互式测试菜单 | 同一台机器测试 |
| `run_with_network_interface.sh` | 指定网络接口运行 | 跨机器通信（推荐） |
| `setup_firewall.sh` | 配置防火墙 | 跨机器通信必需 |

### 诊断工具

| 工具 | 用途 |
|------|------|
| `diagnose_lan_communication.sh` | 完整诊断工具（推荐） |
| `troubleshoot_network.sh` | 网络故障排查 |
| `check_network.sh` | 基础网络检查 |

### 使用示例

```bash
# 1. 诊断网络问题
./diagnose_lan_communication.sh

# 2. 故障排查（需要对方 IP）
sudo ./troubleshoot_network.sh wlp2s0 192.168.1.101

# 3. 配置防火墙
sudo ./setup_firewall.sh
```

---

## 🔧 故障排查

### 问题 1：跨机器无法通信

**快速诊断：**

```bash
./diagnose_lan_communication.sh
```

**常见原因：**

1. ❌ 防火墙未配置
   ```bash
   sudo ./setup_firewall.sh
   ```

2. ❌ 只在一台机器配置了防火墙
   ```bash
   # 两台机器都要执行
   sudo ./setup_firewall.sh
   ```

3. ❌ 网络不通
   ```bash
   ping <对方IP>
   ```

4. ❌ 路由器启用了 AP 隔离
   - 登录路由器管理界面
   - 关闭 "AP 隔离" 或 "客户端隔离"

### 问题 2：组播不工作

**症状：** 关闭防火墙才能通信

**原因：** 防火墙阻止了组播（239.255.0.1）

**解决：** 使用更新后的防火墙配置脚本

```bash
sudo ./setup_firewall.sh
```

脚本会自动配置：
- DDS 端口（7400-7419/udp）
- 组播地址（224.0.0.0/4）
- 本地网段

### 问题 3：程序无法找到

**症状：** `./build/bin/lowcmd_publisher: No such file or directory`

**原因：** 程序未编译或架构不匹配

**解决：**

```bash
cd /mine/Code/unitree/unitree_sdk2
./build.sh
```

编译产物位于：`build_<架构>/bin/`（如 `build_x86_64/bin/`）

### 临时测试：关闭防火墙

```bash
# 两台机器都执行
sudo ufw disable

# 测试通信...

# 测试完成后重新启用
sudo ufw enable
sudo ./setup_firewall.sh
```

---

## 📚 详细文档

### 核心文档

| 文档 | 内容 |
|------|------|
| `docs/跨机器通信指南.md` | 完整的跨机器通信配置步骤 |
| `docs/DDS配置说明.md` | CycloneDDS 配置详解 |
| `docs/故障排查指南.md` | 常见问题和解决方案 |

### 技术文档

| 文档 | 内容 |
|------|------|
| `docs/网络接口问题说明.md` | 网络接口隔离原理 |
| `docs/环境变量配置.md` | DDS 环境变量说明 |

---

## 🎯 最佳实践

### 1. 开发流程

```bash
# 在开发机（x86_64）上开发
./build.sh
./run_test.sh  # 本地测试

# 提交代码
git commit && git push

# 在目标设备（如 aarch64）上部署
git pull
./build.sh
./run_with_network_interface.sh eth0 publisher
```

### 2. 测试流程

**同一台机器：**
```bash
./run_test.sh
```

**跨机器：**
```bash
# 步骤 1：两台都配置防火墙
sudo ./setup_firewall.sh

# 步骤 2：先启动订阅者
机器A: ./run_with_network_interface.sh wlp2s0 subscriber

# 步骤 3：再启动发布者
机器B: ./run_with_network_interface.sh wlp2s0 publisher
```

### 3. 诊断流程

```bash
# 1. 运行完整诊断
./diagnose_lan_communication.sh

# 2. 根据诊断结果修复问题

# 3. 如果还是不通，查看详细日志
sudo tcpdump -i wlp2s0 -n 'udp portrange 7400-7419' -v
```

---

## 💡 提示

1. **网络接口隔离**：在同一台机器上，DDS 会使用共享内存或 localhost，无法通过网络接口实现真正隔离。要测试网络隔离，必须在不同机器上运行。

2. **防火墙配置**：跨机器通信时，两台机器都必须配置防火墙，缺一不可。

3. **Tailscale VPN**：如果使用 Tailscale，需要使用 `quick_test_tailscale.sh` 脚本，因为 Tailscale 不支持组播。

4. **多架构编译**：编译产物会根据系统架构保存到独立目录（`build_x86_64/`、`build_aarch64/`），避免相互覆盖。

---

## 🔗 快速链接

- [项目主 README](../../README.md)
- [架构支持说明](../../架构支持说明.md)
- [GO2 示例文档](../../GO2_DEMO_README.md)

---

## ❓ 常见问题

**Q: 为什么在同一台机器上测试时，网络接口参数无效？**

A: DDS 在同一台机器上会自动使用共享内存或 localhost 优化，绕过网络接口。这是设计行为，不是 bug。

**Q: 跨机器通信需要配置哪些东西？**

A: 
1. 两台机器都运行 `sudo ./setup_firewall.sh`
2. 使用相同的网络接口类型（都用物理网卡或都用 VPN）
3. 确保能互相 ping 通

**Q: 如何验证通信成功？**

A: 订阅者端会输出：
```
接收到 LowCmd 消息 #1
时间戳: 1763797005795 ms
...
```

发布者端会输出：
```
开始发布 LowCmd 消息...
已发布 100 条消息
已发布 200 条消息
...
```

**Q: 编译产物在哪里？**

A: `build_<架构>/bin/`（如 `build_x86_64/bin/`），或通过软链接 `build/bin/` 访问。

---

## 📧 支持

如有问题，请查看：
1. 运行 `./diagnose_lan_communication.sh` 进行自动诊断
2. 查看详细文档：`docs/` 目录
3. 检查防火墙配置：`sudo ufw status`
4. 监控网络流量：`sudo tcpdump -i wlp2s0 -n 'udp portrange 7400-7419'`
