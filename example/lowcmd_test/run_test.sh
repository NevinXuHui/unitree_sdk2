#!/bin/bash

# LowCmd 测试脚本

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
SDK_ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"
BIN_DIR="$SDK_ROOT/build/bin"

echo "========================================"
echo "LowCmd 发布订阅测试"
echo "========================================"
echo "SDK 路径: $SDK_ROOT"
echo "可执行文件路径: $BIN_DIR"
echo ""

# 检查可执行文件是否存在
if [ ! -f "$BIN_DIR/lowcmd_publisher" ] || [ ! -f "$BIN_DIR/lowcmd_subscriber" ]; then
    echo "❌ 错误: 找不到可执行文件"
    echo "请先编译项目："
    echo "  cd $SDK_ROOT"
    echo "  ./build.sh"
    exit 1
fi

# 显示菜单
echo "请选择要运行的程序："
echo "  1) 发布者 (lowcmd_publisher)"
echo "  2) 订阅者 (lowcmd_subscriber)"
echo "  3) 同时运行（在新终端中）"
echo "  4) 退出"
echo ""
read -p "请输入选项 [1-4]: " choice

case $choice in
    1)
        echo ""
        echo "启动发布者..."
        "$BIN_DIR/lowcmd_publisher"
        ;;
    2)
        echo ""
        echo "启动订阅者..."
        "$BIN_DIR/lowcmd_subscriber"
        ;;
    3)
        echo ""
        echo "同时启动发布者和订阅者（需要 gnome-terminal 或 xterm）..."
        
        # 尝试使用 gnome-terminal
        if command -v gnome-terminal &> /dev/null; then
            gnome-terminal -- bash -c "cd '$SDK_ROOT'; echo '订阅者窗口'; '$BIN_DIR/lowcmd_subscriber'; exec bash"
            sleep 1
            gnome-terminal -- bash -c "cd '$SDK_ROOT'; echo '发布者窗口'; '$BIN_DIR/lowcmd_publisher'; exec bash"
        # 尝试使用 xterm
        elif command -v xterm &> /dev/null; then
            xterm -e "cd '$SDK_ROOT'; echo '订阅者窗口'; '$BIN_DIR/lowcmd_subscriber'; bash" &
            sleep 1
            xterm -e "cd '$SDK_ROOT'; echo '发布者窗口'; '$BIN_DIR/lowcmd_publisher'; bash" &
        else
            echo "❌ 错误: 未找到 gnome-terminal 或 xterm"
            echo "请手动在两个终端中分别运行："
            echo "  终端 1: $BIN_DIR/lowcmd_subscriber"
            echo "  终端 2: $BIN_DIR/lowcmd_publisher"
            exit 1
        fi
        
        echo "✅ 已在新终端中启动程序"
        echo "按 Ctrl+C 退出各个程序"
        ;;
    4)
        echo "退出"
        exit 0
        ;;
    *)
        echo "❌ 无效选项"
        exit 1
        ;;
esac
