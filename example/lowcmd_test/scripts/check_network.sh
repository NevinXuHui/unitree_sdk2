#!/bin/bash
# DDS 局域网通信诊断脚本

echo "======================================"
echo "DDS 局域网通信诊断工具"
echo "======================================"

echo ""
echo "1. 检查网络接口："
ip addr show | grep -E "^[0-9]+:|inet "

echo ""
echo "2. 检查防火墙状态："
if command -v ufw &> /dev/null; then
    echo "UFW 防火墙："
    sudo ufw status
elif command -v firewall-cmd &> /dev/null; then
    echo "firewalld 防火墙："
    sudo firewall-cmd --list-all
else
    echo "未检测到防火墙管理工具"
fi

echo ""
echo "3. 检查 DDS 端口占用："
netstat -ulnp 2>/dev/null | grep -E "740[0-9]|741[0-9]" || echo "未检测到 DDS 端口占用"

echo ""
echo "4. 检查组播支持："
ip maddress show

echo ""
echo "======================================"
echo "建议操作："
echo "1. 如果防火墙开启，执行以下命令开放 DDS 端口："
echo "   sudo ufw allow 7400:7419/udp"
echo ""
echo "2. 运行程序时指定正确的网络接口："
echo "   ./build/bin/lowcmd_publisher <网络接口名>"
echo ""
echo "3. 如果在同一台机器上测试，确认是否需要跨机器通信"
echo "======================================"
