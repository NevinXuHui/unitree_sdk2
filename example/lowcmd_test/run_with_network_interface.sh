#!/bin/bash

# LowCmd 测试程序 - 正确配置网络接口
# 使用 CYCLONEDDS_URI 环境变量来确保网络接口隔离真正生效

# 检查参数
if [ $# -lt 2 ]; then
    echo "用法: $0 <network_interface> <publisher|subscriber>"
    echo ""
    echo "示例:"
    echo "  终端 1: $0 eth0 subscriber"
    echo "  终端 2: $0 eth0 publisher"
    echo ""
    echo "可用网络接口:"
    ip addr show | grep -E '^[0-9]+:' | awk '{print "  -", $2}' | sed 's/:$//'
    exit 1
fi

NETWORK_INTERFACE=$1
PROGRAM_TYPE=$2

# 验证网络接口是否存在
if ! ip addr show "$NETWORK_INTERFACE" &> /dev/null; then
    echo "错误: 网络接口 '$NETWORK_INTERFACE' 不存在"
    echo ""
    echo "可用网络接口:"
    ip addr show | grep -E '^[0-9]+:' | awk '{print "  -", $2}' | sed 's/:$//'
    exit 1
fi

# 获取脚本所在目录
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
SDK_ROOT="$SCRIPT_DIR/../.."
BIN_DIR="$SDK_ROOT/build/bin"

# 配置 CycloneDDS 使用指定的网络接口
# 注意：
# 1. AllowMulticast=spdp: 仅在 SPDP（服务发现）中使用组播，数据传输使用单播
# 2. 在同一台机器上，DDS 会自动使用共享内存或 localhost 进行通信
# 3. 网络接口配置主要影响跨机器通信
export CYCLONEDDS_URI="<CycloneDDS><Domain><General><AllowMulticast>spdp</AllowMulticast><Interfaces><NetworkInterface name=\"$NETWORK_INTERFACE\" priority=\"default\" multicast=\"default\"/></Interfaces></General></Domain></CycloneDDS>"

echo "========================================"
echo "LowCmd 测试程序 - 网络接口配置"
echo "========================================"
echo "网络接口: $NETWORK_INTERFACE"
echo "程序类型: $PROGRAM_TYPE"
echo "DDS 配置: 已通过 CYCLONEDDS_URI 设置"
echo ""
echo "⚠️  重要提示："
echo "   在同一台机器上运行的发布者和订阅者"
echo "   会通过共享内存或 localhost 通信，"
echo "   无法通过网络接口实现真正的隔离。"
echo "   要测试网络隔离，请在不同机器上运行。"
echo "========================================"
echo ""

# 运行对应的程序
case "$PROGRAM_TYPE" in
    publisher|pub)
        echo "启动发布者..."
        "$BIN_DIR/lowcmd_publisher"
        ;;
    subscriber|sub)
        echo "启动订阅者..."
        "$BIN_DIR/lowcmd_subscriber"
        ;;
    *)
        echo "错误: 程序类型必须是 'publisher' 或 'subscriber'"
        exit 1
        ;;
esac
