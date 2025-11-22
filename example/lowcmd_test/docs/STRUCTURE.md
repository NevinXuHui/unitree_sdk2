# 目录结构

```
lowcmd_test/
│
├── 📖 主文档（必读）
│   ├── README.md                      # 快速开始指南
│   ├── QUICK_REFERENCE.md             # 快速参考卡片
│   └── STRUCTURE.md                   # 本文件（目录说明）
│
├── 🎮 核心脚本（常用）
│   ├── run_test.sh                    # 本地测试（同一台机器）
│   ├── run_with_network_interface.sh  # 跨机器通信（推荐）
│   ├── setup_firewall.sh              # 防火墙配置（跨机器必需）
│   └── diagnose_lan_communication.sh  # 诊断工具（问题排查）
│
├── 🔧 辅助脚本（较少使用）
│   ├── troubleshoot_network.sh        # 网络故障排查
│   └── quick_test_tailscale.sh        # Tailscale VPN 测试
│
├── 💻 源代码
│   ├── lowcmd_publisher.cpp           # 发布者源码
│   ├── lowcmd_subscriber.cpp          # 订阅者源码
│   └── CMakeLists.txt                 # 编译配置
│
├── 📚 docs/（详细文档）
│   ├── README.md                      # 文档目录说明
│   ├── INDEX.md                       # 文档索引（查找文档）
│   ├── 跨机器通信配置清单.md          # 完整配置步骤
│   ├── 局域网通信问题解决方案.md      # 故障排查指南
│   ├── DDS配置说明.md                 # DDS 配置详解
│   ├── network_interface_issue.md     # 网络接口原理
│   ├── 环境变量标准格式.md            # 环境变量说明
│   └── 环境变量更新说明.txt           # 更新历史
│
└── 🛠️ scripts/（高级工具）
    ├── README.md                      # 脚本目录说明
    ├── check_network.sh               # 基础网络检查
    ├── generate_dds_config.sh         # 配置生成工具
    ├── test_config.sh                 # 配置测试
    ├── cyclonedds_eth10.xml           # 配置示例（eth10）
    └── cyclonedds_tailscale_unicast.xml  # 配置示例（Tailscale）
```

## 快速导航

### 📖 想要开始使用？
→ 阅读 `README.md`

### 🌐 需要跨机器通信？
1. `sudo ./setup_firewall.sh`（两台机器都要）
2. `./run_with_network_interface.sh wlp2s0 subscriber`（机器 A）
3. `./run_with_network_interface.sh wlp2s0 publisher`（机器 B）

### 🔍 遇到问题？
→ 运行 `./diagnose_lan_communication.sh`

### 📚 需要详细文档？
→ 查看 `docs/INDEX.md`

## 文件功能速查

| 文件 | 功能 | 何时使用 |
|------|------|----------|
| `README.md` | 主文档 | 第一次使用 |
| `run_test.sh` | 本地测试 | 同一台机器测试 |
| `run_with_network_interface.sh` | 跨机器通信 | 不同机器测试 |
| `setup_firewall.sh` | 配置防火墙 | 跨机器通信前 |
| `diagnose_lan_communication.sh` | 诊断工具 | 通信失败时 |
| `docs/INDEX.md` | 文档索引 | 查找详细文档 |

## 脚本使用频率

**⭐⭐⭐ 常用：**
- `run_test.sh` - 本地测试
- `run_with_network_interface.sh` - 跨机器通信
- `setup_firewall.sh` - 防火墙配置

**⭐⭐ 较常用：**
- `diagnose_lan_communication.sh` - 问题诊断

**⭐ 偶尔用：**
- `troubleshoot_network.sh` - 详细排查
- `quick_test_tailscale.sh` - Tailscale VPN
- `scripts/*` - 高级配置工具

## 文档阅读顺序

1. **README.md** - 必读
2. **docs/跨机器通信配置清单.md** - 跨机器通信时读
3. **docs/局域网通信问题解决方案.md** - 遇到问题时读
4. **docs/DDS配置说明.md** - 需要自定义配置时读
5. **其他文档** - 按需查阅

## 简化原则

✅ **核心脚本** - 放在根目录，常用
✅ **文档** - 放在 docs/，详细说明
✅ **辅助工具** - 放在 scripts/，高级用户使用
✅ **命名清晰** - 一看就懂用途
✅ **README 简洁** - 快速上手
✅ **INDEX 完整** - 查找方便
