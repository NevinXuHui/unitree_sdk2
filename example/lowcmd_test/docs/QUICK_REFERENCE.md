# 快速参考

## 🚀 最常用命令

### 本地测试（同一台机器）
```bash
./run_test.sh
```

### 跨机器通信

**机器 A（订阅者）:**
```bash
sudo ./setup_firewall.sh
./run_with_network_interface.sh wlp2s0 subscriber
```

**机器 B（发布者）:**
```bash
sudo ./setup_firewall.sh
./run_with_network_interface.sh wlp2s0 publisher
```

### 问题诊断
```bash
./diagnose_lan_communication.sh
```

---

## 🔧 故障排查速查表

| 症状 | 可能原因 | 解决方法 |
|------|----------|----------|
| 跨机器无法通信 | 防火墙未配置 | `sudo ./setup_firewall.sh` |
| 关闭防火墙才能通信 | 防火墙规则不完整 | 重新运行 `setup_firewall.sh` |
| ping 不通对方 | 网络不通或 AP 隔离 | 检查网络、关闭 AP 隔离 |
| 程序无法找到 | 未编译 | `cd ../.. && ./build.sh` |
| 同机器网络接口无效 | DDS 使用共享内存 | 正常现象，跨机器测试 |

---

## 📋 防火墙规则速查

### 检查防火墙状态
```bash
sudo ufw status
```

### 手动添加规则（如果脚本失败）
```bash
# DDS 端口
sudo ufw allow 7400:7419/udp

# 组播（关键！）
sudo ufw allow from 224.0.0.0/4

# 本地网段（替换为实际网段）
sudo ufw allow from 192.168.1.0/24
```

### 临时关闭防火墙测试
```bash
sudo ufw disable
# 测试...
sudo ufw enable
```

---

## 🌐 网络接口速查

### 查看所有接口
```bash
ip addr show
```

### 查看特定接口
```bash
ip addr show wlp2s0
```

### 常见接口名
- `eth0`, `eth1` - 有线网卡
- `wlp2s0`, `wlan0` - 无线网卡
- `enp3s0` - 以太网
- `tailscale0` - Tailscale VPN

---

## 📊 监控和调试

### 监控 DDS 流量
```bash
sudo tcpdump -i wlp2s0 -n 'udp portrange 7400-7419' -v
```

### 监控组播流量
```bash
sudo tcpdump -i wlp2s0 -n dst 239.255.0.1
```

### 查看 DDS 端口监听
```bash
sudo netstat -ulnp | grep -E "740[0-9]|741[0-9]"
```

### 查看进程
```bash
ps aux | grep lowcmd
```

### 杀死进程
```bash
pkill -f lowcmd_publisher
pkill -f lowcmd_subscriber
```

---

## 🔑 环境变量

### 查看当前 DDS 配置
```bash
echo $CYCLONEDDS_URI
```

### 手动设置（物理网卡）
```bash
export CYCLONEDDS_URI='<CycloneDDS><Domain><General><AllowMulticast>spdp</AllowMulticast><Interfaces><NetworkInterface name="wlp2s0" priority="default" multicast="default"/></Interfaces></General></Domain></CycloneDDS>'
```

### 手动设置（Tailscale VPN）
```bash
export CYCLONEDDS_URI='<CycloneDDS><Domain><General><AllowMulticast>false</AllowMulticast><Interfaces><NetworkInterface name="tailscale0" priority="default" multicast="false"/></Interfaces></General></Domain></CycloneDDS>'
```

---

## 📁 文件位置

### 编译产物
```bash
# 当前架构
../../build/bin/lowcmd_publisher
../../build/bin/lowcmd_subscriber

# 特定架构
../../build_x86_64/bin/lowcmd_publisher
../../build_aarch64/bin/lowcmd_publisher
```

### 配置文件
```bash
scripts/cyclonedds_eth10.xml           # eth10 配置示例
scripts/cyclonedds_tailscale_unicast.xml  # Tailscale 配置
```

### 文档
```bash
README.md                               # 主文档
STRUCTURE.md                            # 目录结构
docs/INDEX.md                           # 文档索引
docs/跨机器通信配置清单.md              # 配置步骤
docs/局域网通信问题解决方案.md          # 故障排查
```

---

## ⚡ 一键命令

### 完整测试流程（本地）
```bash
cd /mine/Code/unitree/unitree_sdk2 && \
./build.sh && \
cd example/lowcmd_test && \
./run_test.sh
```

### 跨机器配置（机器 A）
```bash
cd /mine/Code/unitree/unitree_sdk2/example/lowcmd_test && \
sudo ./setup_firewall.sh && \
./run_with_network_interface.sh wlp2s0 subscriber
```

### 跨机器配置（机器 B）
```bash
cd /mine/Code/unitree/unitree_sdk2/example/lowcmd_test && \
sudo ./setup_firewall.sh && \
./run_with_network_interface.sh wlp2s0 publisher
```

### 完整诊断
```bash
./diagnose_lan_communication.sh && \
sudo ./troubleshoot_network.sh wlp2s0 <对方IP>
```

---

## 📞 快速帮助

```bash
# 脚本帮助
./run_with_network_interface.sh --help

# 查看文档
cat README.md
cat STRUCTURE.md
cat docs/INDEX.md

# 运行诊断
./diagnose_lan_communication.sh

# 查看日志
sudo tcpdump -i wlp2s0 -n 'udp portrange 7400-7419'
```

---

## 💡 记住这些

1. **两台机器都要配置防火墙**
2. **先启动订阅者，再启动发布者**
3. **同一台机器测试用 `run_test.sh`**
4. **跨机器测试用 `run_with_network_interface.sh`**
5. **遇到问题先运行 `diagnose_lan_communication.sh`**
6. **组播支持是关键**（224.0.0.0/4）

---

## 🎯 最小配置

只需要这 3 步：

1. **编译**: `./build.sh`
2. **防火墙**: `sudo ./setup_firewall.sh`（两台机器）
3. **运行**: `./run_with_network_interface.sh wlp2s0 <pub/sub>`

完成！
