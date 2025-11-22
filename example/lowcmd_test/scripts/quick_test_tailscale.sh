#!/bin/bash
# Tailscale 快速测试脚本

echo "========================================"
echo "Tailscale DDS 快速测试向导"
echo "========================================"
echo ""

# 检查 Tailscale 接口
if ! ip addr show tailscale0 &> /dev/null; then
    echo "❌ 错误: Tailscale 接口不存在！"
    echo ""
    echo "请先安装并启动 Tailscale："
    echo "  https://tailscale.com/download"
    exit 1
fi

# 获取本机 Tailscale IP
LOCAL_IP=$(ip addr show tailscale0 | grep "inet " | awk '{print $2}' | cut -d'/' -f1)
echo "✅ Tailscale 已连接"
echo "本机 Tailscale IP: $LOCAL_IP"
echo ""

# 检查防火墙
echo "检查防火墙配置..."
if command -v ufw &> /dev/null; then
    if sudo ufw status | grep -q "7400:7419"; then
        echo "✅ 防火墙已配置"
    else
        echo "⚠️  防火墙未配置，正在配置..."
        sudo ufw allow 7400:7419/udp
        echo "✅ 防火墙配置完成"
    fi
fi
echo ""

# 显示远程机器列表
echo "可用的 Tailscale 节点："
echo "========================================"
tailscale status | grep -v "^#" | head -10
echo ""

# 获取用户输入
read -p "请输入远程机器的 Tailscale IP（或按回车跳过）: " REMOTE_IP
echo ""

# 选择运行模式
echo "请选择运行模式："
echo "  1) 订阅者（接收消息）"
echo "  2) 发布者（发送消息）"
echo ""
read -p "请选择 [1-2]: " MODE
echo ""

# 设置 DDS 配置（禁用组播）
export CYCLONEDDS_URI='<CycloneDDS><Domain><General><AllowMulticast>false</AllowMulticast><Interfaces><NetworkInterface name="tailscale0" priority="default" multicast="false"/></Interfaces></General></Domain></CycloneDDS>'

echo "========================================"
echo "DDS 配置："
echo "  网络接口: tailscale0"
echo "  组播模式: 禁用（Tailscale 不支持）"
echo "  本机 IP: $LOCAL_IP"
if [ -n "$REMOTE_IP" ]; then
    echo "  对等节点: $REMOTE_IP"
fi
echo "========================================"
echo ""

# 获取脚本所在目录和架构
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
SDK_ROOT="$SCRIPT_DIR/../.."
ARCH=$(uname -m)
BIN_DIR="$SDK_ROOT/build_${ARCH}/bin"

# 检查架构特定的构建目录是否存在，否则使用通用构建目录
if [ ! -d "$BIN_DIR" ]; then
    BIN_DIR="$SDK_ROOT/build/bin"
fi

case $MODE in
    1)
        echo "启动订阅者..."
        echo ""
        echo "⚠️  请在远程机器上运行："
        echo "  ssh user@<远程机器>"
        echo "  cd $SCRIPT_DIR"
        echo "  ./quick_test_tailscale.sh"
        echo "  选择模式: 2 (发布者)"
        echo ""
        echo "======================================"
        "$BIN_DIR/lowcmd_subscriber"
        ;;
    2)
        echo "启动发布者..."
        echo ""
        echo "⚠️  确保远程机器上已启动订阅者"
        echo ""
        echo "======================================"
        "$BIN_DIR/lowcmd_publisher"
        ;;
    *)
        echo "❌ 无效选项"
        exit 1
        ;;
esac
