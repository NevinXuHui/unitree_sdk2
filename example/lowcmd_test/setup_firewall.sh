#!/bin/bash
# DDS 防火墙配置脚本

echo "======================================"
echo "DDS 防火墙配置工具"
echo "======================================"

# 检测防火墙类型
if command -v ufw &> /dev/null; then
    echo "检测到 UFW 防火墙，正在配置..."
    echo ""
    
    # 开放 DDS 端口
    echo "开放 UDP 端口 7400-7419..."
    sudo ufw allow 7400:7419/udp
    
    echo ""
    echo "配置完成！当前防火墙状态："
    sudo ufw status
    
elif command -v firewall-cmd &> /dev/null; then
    echo "检测到 firewalld 防火墙，正在配置..."
    echo ""
    
    # 开放 DDS 端口
    echo "开放 UDP 端口 7400-7419..."
    sudo firewall-cmd --permanent --add-port=7400-7419/udp
    sudo firewall-cmd --reload
    
    echo ""
    echo "配置完成！当前防火墙状态："
    sudo firewall-cmd --list-all
    
else
    echo "未检测到支持的防火墙管理工具 (ufw 或 firewalld)"
    echo ""
    echo "请手动配置防火墙开放以下端口："
    echo "  - UDP 7400-7419"
fi

echo ""
echo "======================================"
echo "提示："
echo "1. 配置完成后，请在两台机器上运行程序测试"
echo "2. 发布者：./build/bin/lowcmd_publisher <网络接口>"
echo "3. 订阅者：./build/bin/lowcmd_subscriber <网络接口>"
echo "======================================"
