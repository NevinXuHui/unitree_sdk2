#!/bin/bash
# DDS 局域网通信完整诊断工具

echo "========================================"
echo "DDS 局域网通信诊断工具"
echo "========================================"
echo ""

# 1. 检查本机网络配置
echo "【1. 本机网络配置】"
echo "网络接口: wlp2s0"
LOCAL_IP=$(ip addr show wlp2s0 2>/dev/null | grep "inet " | awk '{print $2}' | cut -d'/' -f1)
if [ -z "$LOCAL_IP" ]; then
    echo "❌ wlp2s0 接口不存在或无 IP"
    exit 1
fi
echo "本机 IP: $LOCAL_IP"

LOCAL_SUBNET=$(ip -o -f inet addr show wlp2s0 | awk '{print $4}')
echo "本机网段: $LOCAL_SUBNET"
echo ""

# 2. 检查防火墙
echo "【2. 防火墙检查】"
if command -v ufw &> /dev/null; then
    UFW_STATUS=$(sudo ufw status | head -1)
    echo "UFW 状态: $UFW_STATUS"
    
    if sudo ufw status | grep -q "7400:7419/udp"; then
        echo "✅ DDS 端口已开放"
    else
        echo "❌ DDS 端口未开放"
        echo "   运行: sudo ufw allow 7400:7419/udp"
    fi
    
    if sudo ufw status | grep -q "224.0.0.0/4"; then
        echo "✅ 组播已允许"
    else
        echo "❌ 组播未允许"
        echo "   运行: sudo ufw allow from 224.0.0.0/4"
    fi
    
    SUBNET_BASE=$(echo $LOCAL_SUBNET | cut -d'.' -f1-3)
    if sudo ufw status | grep -q "$SUBNET_BASE"; then
        echo "✅ 本地网段已允许"
    else
        echo "❌ 本地网段未允许"
        echo "   运行: sudo ufw allow from ${SUBNET_BASE}.0/24"
    fi
fi
echo ""

# 3. 检查远程机器连通性
echo "【3. 远程机器连通性】"
read -p "请输入远程机器的 IP 地址（按回车跳过）: " REMOTE_IP
echo ""

if [ -n "$REMOTE_IP" ]; then
    echo "测试与远程机器 $REMOTE_IP 的连通性..."
    if ping -c 3 -W 2 "$REMOTE_IP" &> /dev/null; then
        echo "✅ 可以 ping 通远程机器"
    else
        echo "❌ 无法 ping 通远程机器"
        echo "   可能原因："
        echo "   1. 远程机器不在线"
        echo "   2. 远程机器防火墙阻止 ICMP"
        echo "   3. 不在同一个网络"
    fi
    echo ""
    
    # 测试 DDS 端口
    echo "测试 DDS 端口连通性..."
    if command -v nc &> /dev/null; then
        if nc -zvu "$REMOTE_IP" 7410 -w 2 2>&1 | grep -q "succeeded\|open"; then
            echo "✅ 可以连接到远程 DDS 端口"
        else
            echo "⚠️  无法连接到远程 DDS 端口（可能是 UDP 端口无法用 nc 测试）"
        fi
    fi
    echo ""
fi

# 4. 检查 DDS 配置
echo "【4. DDS 配置检查】"
if [ -n "$CYCLONEDDS_URI" ]; then
    echo "✅ CYCLONEDDS_URI 已设置"
    echo "当前配置:"
    echo "$CYCLONEDDS_URI" | grep -o '<[^>]*>' | head -5
else
    echo "❌ CYCLONEDDS_URI 未设置"
    echo "   使用脚本运行会自动设置"
fi
echo ""

# 5. 检查进程
echo "【5. 进程检查】"
if pgrep -f lowcmd_publisher > /dev/null; then
    echo "✅ 发布者进程正在运行"
    echo "   PID: $(pgrep -f lowcmd_publisher)"
else
    echo "⚪ 发布者进程未运行"
fi

if pgrep -f lowcmd_subscriber > /dev/null; then
    echo "✅ 订阅者进程正在运行"
    echo "   PID: $(pgrep -f lowcmd_subscriber)"
else
    echo "⚪ 订阅者进程未运行"
fi
echo ""

# 6. 检查 DDS 端口监听
echo "【6. DDS 端口监听】"
DDS_PORTS=$(sudo netstat -ulnp 2>/dev/null | grep -E "740[0-9]|741[0-9]" | head -5)
if [ -n "$DDS_PORTS" ]; then
    echo "✅ DDS 端口正在监听:"
    echo "$DDS_PORTS" | awk '{print "   "$4, "->", $NF}'
else
    echo "❌ 没有 DDS 端口在监听"
    echo "   请先启动发布者或订阅者"
fi
echo ""

# 7. 监控组播流量
echo "【7. 组播流量监控】"
echo "是否要监控组播流量？(需要 sudo 权限，按 Ctrl+C 停止)"
read -p "按回车开始，或输入 n 跳过: " MONITOR
if [ "$MONITOR" != "n" ] && [ "$MONITOR" != "N" ]; then
    echo ""
    echo "监控组播流量 (239.255.0.1)，按 Ctrl+C 停止..."
    timeout 10 sudo tcpdump -i wlp2s0 -n dst 239.255.0.1 2>/dev/null || echo "⚠️  未捕获到组播流量"
fi
echo ""

# 8. 建议
echo "========================================"
echo "【诊断建议】"
echo "========================================"
echo ""

if [ -z "$REMOTE_IP" ]; then
    echo "⚠️  未提供远程机器 IP，无法测试连通性"
    echo ""
fi

echo "✅ 确保两台机器都执行以下操作："
echo ""
echo "1. 配置防火墙（两台机器都要）:"
echo "   cd /mine/Code/unitree/unitree_sdk2/example/lowcmd_test"
echo "   sudo ./setup_firewall.sh"
echo ""
echo "2. 在机器 A（订阅者）上运行:"
echo "   ./run_with_network_interface.sh wlp2s0 subscriber"
echo ""
echo "3. 在机器 B（发布者）上运行:"
echo "   ./run_with_network_interface.sh wlp2s0 publisher"
echo ""
echo "4. 如果还是不通，尝试临时关闭防火墙测试:"
echo "   sudo ufw disable"
echo "   测试完成后记得重新启用: sudo ufw enable"
echo ""
echo "5. 检查路由器是否启用了 AP 隔离"
echo ""
echo "========================================"
