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
    
    # 允许组播（关键！）
    echo "允许组播通信..."
    sudo ufw allow from 224.0.0.0/4
    
    # 允许本地网络通信（如果在局域网）
    LOCAL_SUBNET=$(ip route | grep -E 'scope link' | head -1 | awk '{print $1}')
    if [ -n "$LOCAL_SUBNET" ]; then
        echo "允许本地网络 ${LOCAL_SUBNET}..."
        sudo ufw allow from ${LOCAL_SUBNET}
    fi
    
    echo ""
    echo "配置完成！当前防火墙状态："
    sudo ufw status numbered
    
elif command -v firewall-cmd &> /dev/null; then
    echo "检测到 firewalld 防火墙，正在配置..."
    echo ""
    
    # 开放 DDS 端口
    echo "开放 UDP 端口 7400-7419..."
    sudo firewall-cmd --permanent --add-port=7400-7419/udp
    
    # 允许组播
    echo "允许组播通信..."
    sudo firewall-cmd --permanent --add-rich-rule='rule family="ipv4" source address="224.0.0.0/4" accept'
    
    # 允许本地网络
    LOCAL_ZONE=$(sudo firewall-cmd --get-default-zone)
    echo "设置本地网络为信任区域: ${LOCAL_ZONE}"
    sudo firewall-cmd --permanent --zone=${LOCAL_ZONE} --add-source=192.168.0.0/16
    sudo firewall-cmd --permanent --zone=${LOCAL_ZONE} --add-source=10.0.0.0/8
    
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
