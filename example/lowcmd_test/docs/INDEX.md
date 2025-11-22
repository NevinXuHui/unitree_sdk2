# LowCmd 测试程序文档索引

## 📖 文档结构

```
lowcmd_test/
├── README.md                          # 主文档（快速开始）
├── docs/                              # 详细文档
│   ├── INDEX.md                      # 本文件（文档索引）
│   ├── 跨机器通信配置清单.md          # 完整配置步骤
│   ├── 局域网通信问题解决方案.md      # 故障排查指南
│   ├── DDS配置说明.md                # DDS 配置详解
│   ├── network_interface_issue.md    # 网络接口原理
│   ├── 环境变量标准格式.md            # 环境变量说明
│   └── 环境变量更新说明.txt          # 更新历史
├── scripts/                           # 辅助脚本
│   ├── check_network.sh              # 基础网络检查
│   ├── generate_dds_config.sh        # 配置生成工具
│   ├── test_config.sh                # 配置测试
│   ├── cyclonedds_eth10.xml          # 示例配置
│   └── cyclonedds_tailscale_unicast.xml  # Tailscale 配置
└── (核心脚本在根目录)

核心脚本：
├── run_test.sh                        # 本地测试
├── run_with_network_interface.sh     # 跨机器通信
├── setup_firewall.sh                 # 防火墙配置
├── diagnose_lan_communication.sh     # 诊断工具
├── troubleshoot_network.sh           # 故障排查
└── quick_test_tailscale.sh           # Tailscale 测试
```

---

## 🚀 快速导航

### 新手入门

1. **[主 README](../README.md)** - 从这里开始
2. **运行脚本** - `./run_test.sh`（本地测试）
3. **配置防火墙** - `sudo ./setup_firewall.sh`（跨机器必需）

### 跨机器通信

1. **[跨机器通信配置清单](跨机器通信配置清单.md)** - 完整步骤
2. **[局域网通信问题解决方案](局域网通信问题解决方案.md)** - 故障排查
3. **运行诊断** - `./diagnose_lan_communication.sh`

### 高级配置

1. **[DDS 配置说明](DDS配置说明.md)** - 深入理解 DDS
2. **[环境变量标准格式](环境变量标准格式.md)** - 配置参数
3. **[网络接口问题说明](network_interface_issue.md)** - 技术原理

---

## 📚 文档详解

### 主文档：README.md

**内容：**
- 快速开始指南
- 跨机器通信配置
- 脚本说明
- 故障排查

**适合：** 所有用户

**快速链接：**
```bash
cat ../README.md
```

---

### 跨机器通信配置清单.md

**内容：**
- 前提条件检查
- 完整配置步骤
- 网络拓扑示例
- 常见配置错误
- 测试清单

**适合：** 需要跨机器通信的用户

**使用场景：**
- 第一次配置跨机器通信
- 通信失败需要系统排查

**快速链接：**
```bash
cat 跨机器通信配置清单.md
```

---

### 局域网通信问题解决方案.md

**内容：**
- 常见问题和解决方案
- 防火墙配置详解
- 网络接口配置
- DDS 通信原理
- 故障排查步骤

**适合：** 遇到通信问题的用户

**使用场景：**
- 跨机器无法通信
- 防火墙配置问题
- 网络隔离问题

**快速链接：**
```bash
cat 局域网通信问题解决方案.md
```

---

### DDS配置说明.md

**内容：**
- CYCLONEDDS_URI 环境变量详解
- 配置方法（环境变量、XML 文件、脚本）
- 配置参数说明
- 故障排查技巧

**适合：** 需要自定义 DDS 配置的用户

**使用场景：**
- 理解 DDS 配置原理
- 自定义网络配置
- 调试 DDS 问题

**快速链接：**
```bash
cat DDS配置说明.md
```

---

### network_interface_issue.md

**内容：**
- 为什么网络接口隔离不工作
- DDS 本地通信机制
- 共享内存和 localhost 优化
- 临时解决方案

**适合：** 想深入理解技术原理的用户

**使用场景：**
- 理解为什么同一台机器上网络接口参数无效
- 学习 DDS 通信机制

**快速链接：**
```bash
cat network_interface_issue.md
```

---

### 环境变量标准格式.md

**内容：**
- CYCLONEDDS_URI 标准格式
- 配置参数详解
- 格式对比（新旧版本）
- 使用示例

**适合：** 需要手动配置环境变量的用户

**使用场景：**
- 手动设置 CYCLONEDDS_URI
- 理解配置参数含义

**快速链接：**
```bash
cat 环境变量标准格式.md
```

---

### 环境变量更新说明.txt

**内容：**
- 环境变量更新历史
- 新旧格式对比
- 受影响的文件列表

**适合：** 升级旧版本配置的用户

**使用场景：**
- 从旧版本迁移
- 了解配置变更历史

**快速链接：**
```bash
cat 环境变量更新说明.txt
```

---

## 🛠️ 脚本详解

### 核心脚本（根目录）

#### run_test.sh
**用途：** 交互式测试菜单

**使用：**
```bash
./run_test.sh
```

**场景：** 同一台机器测试

---

#### run_with_network_interface.sh
**用途：** 指定网络接口运行程序

**使用：**
```bash
./run_with_network_interface.sh <网络接口> <publisher|subscriber>

# 示例
./run_with_network_interface.sh wlp2s0 subscriber
./run_with_network_interface.sh wlp2s0 publisher
```

**场景：** 跨机器通信（推荐）

---

#### setup_firewall.sh
**用途：** 自动配置防火墙

**使用：**
```bash
sudo ./setup_firewall.sh
```

**配置内容：**
- DDS 端口（7400-7419/udp）
- 组播地址（224.0.0.0/4）
- 本地网段

**场景：** 跨机器通信必需（两台机器都要运行）

---

#### diagnose_lan_communication.sh
**用途：** 完整的通信诊断工具

**使用：**
```bash
./diagnose_lan_communication.sh
```

**检查内容：**
- 网络配置
- 防火墙规则
- 远程连通性
- DDS 配置
- 进程状态
- 端口监听

**场景：** 通信问题首选诊断工具

---

#### troubleshoot_network.sh
**用途：** 网络故障排查

**使用：**
```bash
sudo ./troubleshoot_network.sh <网络接口> [远程IP]

# 示例
sudo ./troubleshoot_network.sh wlp2s0 192.168.1.101
```

**场景：** 需要详细网络分析时使用

---

#### quick_test_tailscale.sh
**用途：** Tailscale VPN 快速测试

**使用：**
```bash
./quick_test_tailscale.sh
```

**场景：** 通过 Tailscale VPN 进行跨机器通信

---

### 辅助脚本（scripts/ 目录）

#### check_network.sh
基础网络检查工具

#### generate_dds_config.sh
为任意网络接口生成配置文件

#### test_config.sh
测试 DDS 配置是否正确

#### cyclonedds_*.xml
DDS 配置示例文件

---

## 🎯 使用路径

### 路径 1：本地测试（最简单）

```bash
1. ./run_test.sh
2. 选择要运行的程序
3. 完成
```

**文档：** 主 README.md

---

### 路径 2：跨机器通信（标准流程）

```bash
1. 阅读：跨机器通信配置清单.md
2. 两台机器：sudo ./setup_firewall.sh
3. 机器A：./run_with_network_interface.sh wlp2s0 subscriber
4. 机器B：./run_with_network_interface.sh wlp2s0 publisher
5. 如果失败：./diagnose_lan_communication.sh
```

**文档：**
- 跨机器通信配置清单.md
- 局域网通信问题解决方案.md

---

### 路径 3：故障排查

```bash
1. ./diagnose_lan_communication.sh
2. 根据诊断结果查看对应文档
3. 运行建议的修复命令
4. 如果还不行，查看：局域网通信问题解决方案.md
```

**文档：**
- 局域网通信问题解决方案.md
- DDS配置说明.md

---

### 路径 4：高级配置

```bash
1. 阅读：DDS配置说明.md
2. 阅读：环境变量标准格式.md
3. 使用：scripts/generate_dds_config.sh
4. 测试：scripts/test_config.sh
```

**文档：**
- DDS配置说明.md
- 环境变量标准格式.md
- network_interface_issue.md

---

## 📊 文档优先级

### ⭐⭐⭐ 必读

- **主 README.md** - 所有用户
- **跨机器通信配置清单.md** - 跨机器通信用户

### ⭐⭐ 推荐

- **局域网通信问题解决方案.md** - 遇到问题时
- **DDS配置说明.md** - 需要自定义配置时

### ⭐ 可选

- **network_interface_issue.md** - 想了解原理
- **环境变量标准格式.md** - 手动配置环境变量
- **环境变量更新说明.txt** - 升级旧版本

---

## 🔄 文档更新

**最后更新：** 2025-11-22

**主要变更：**
- 整理文档结构
- 移动文档到 docs/ 目录
- 移动辅助脚本到 scripts/ 目录
- 创建文档索引
- 简化主 README

**历史：**
- 修复防火墙配置（添加组播支持）
- 添加多架构编译支持
- 创建完整诊断工具

---

## 💡 提示

1. **从主 README 开始** - 涵盖 90% 的使用场景
2. **遇到问题先运行诊断** - `./diagnose_lan_communication.sh`
3. **查看文档索引** - 本文件，找到对应的详细文档
4. **先看概述后看详细** - 避免信息过载

---

## 📞 获取帮助

1. **运行诊断工具** - `./diagnose_lan_communication.sh`
2. **查看对应文档** - 根据问题类型
3. **检查防火墙** - `sudo ufw status`
4. **测试网络** - `ping <对方IP>`
5. **查看日志** - `sudo tcpdump -i wlp2s0 -n 'udp portrange 7400-7419'`
