#!/bin/bash
# DDS 局域网通信故障排查工具

echo "========================================"
echo "DDS 局域网通信故障排查工具"
echo "========================================"
echo ""

# 检查参数
if [ $# -eq 0 ]; then
    echo "用法: $0 <网络接口名> [远程机器IP]"
    echo ""
    echo "示例:"
    echo "  $0 tailscale0"
    echo "  $0 tailscale0 100.81.1.100"
    echo ""
    exit 1
fi

INTERFACE=$1
REMOTE_IP=$2

echo "1. 检查网络接口状态"
echo "========================================"
if ip addr show "$INTERFACE" &> /dev/null; then
    echo "✅ 接口 $INTERFACE 存在"
    echo ""
    echo "接口详情："
    ip addr show "$INTERFACE" | grep -E "inet |mtu"
    echo ""
else
    echo "❌ 接口 $INTERFACE 不存在！"
    exit 1
fi

echo "2. 检查防火墙状态"
echo "========================================"
if command -v ufw &> /dev/null; then
    echo "UFW 防火墙状态："
    sudo ufw status | grep -E "Status|7400:7419"
    if ! sudo ufw status | grep -q "7400:7419"; then
        echo ""
        echo "⚠️  警告: DDS 端口 (7400-7419) 未开放！"
        echo "运行以下命令开放端口："
        echo "  sudo ufw allow 7400:7419/udp"
    fi
elif command -v firewall-cmd &> /dev/null; then
    echo "firewalld 防火墙状态："
    sudo firewall-cmd --list-ports | grep -E "7400-7419"
    if ! sudo firewall-cmd --list-ports | grep -q "7400-7419"; then
        echo ""
        echo "⚠️  警告: DDS 端口 (7400-7419) 未开放！"
        echo "运行以下命令开放端口："
        echo "  sudo firewall-cmd --permanent --add-port=7400-7419/udp"
        echo "  sudo firewall-cmd --reload"
    fi
else
    echo "ℹ️  未检测到防火墙管理工具"
fi
echo ""

echo "3. 检查 DDS 端口监听状态"
echo "========================================"
DDS_PORTS=$(netstat -ulnp 2>/dev/null | grep -E "740[0-9]|741[0-9]" | head -5)
if [ -n "$DDS_PORTS" ]; then
    echo "✅ 检测到 DDS 端口活动："
    echo "$DDS_PORTS"
else
    echo "⚠️  未检测到 DDS 端口活动"
    echo "可能原因：DDS 程序未运行或使用了动态端口"
fi
echo ""

echo "4. 检查路由和网关"
echo "========================================"
echo "路由表："
ip route | grep "$INTERFACE" || echo "未找到 $INTERFACE 的路由"
echo ""

echo "5. 检查组播支持"
echo "========================================"
if ip maddress show "$INTERFACE" | grep -q "239.255.0.1"; then
    echo "✅ 接口支持组播"
else
    echo "⚠️  接口可能不支持组播或未加入 DDS 组播组"
fi
echo ""

# 如果提供了远程 IP，进行连通性测试
if [ -n "$REMOTE_IP" ]; then
    echo "6. 测试与远程机器的连通性"
    echo "========================================"
    echo "Ping 测试到 $REMOTE_IP:"
    if ping -c 3 -W 2 "$REMOTE_IP" &> /dev/null; then
        echo "✅ 网络连通"
    else
        echo "❌ 网络不通！请检查网络配置"
    fi
    echo ""
    
    echo "测试 DDS 端口 7410 (UDP):"
    if command -v nc &> /dev/null; then
        nc -vzu -w 2 "$REMOTE_IP" 7410 2>&1 | grep -E "succeeded|open" && echo "✅ 端口可达" || echo "⚠️  端口不可达"
    else
        echo "ℹ️  未安装 nc 工具，跳过端口测试"
    fi
    echo ""
fi

echo "7. 当前 DDS 配置"
echo "========================================"
if [ -n "$CYCLONEDDS_URI" ]; then
    echo "✅ CYCLONEDDS_URI 已设置："
    echo "$CYCLONEDDS_URI" | sed 's/></>\n</g'
else
    echo "⚠️  CYCLONEDDS_URI 未设置"
    echo "建议设置："
    echo "export CYCLONEDDS_URI='<CycloneDDS><Domain><General><AllowMulticast>spdp</AllowMulticast><Interfaces><NetworkInterface name=\"$INTERFACE\" priority=\"default\" multicast=\"default\"/></Interfaces></General></Domain></CycloneDDS>'"
fi
echo ""

echo "8. 特殊网络接口检查"
echo "========================================"
if echo "$INTERFACE" | grep -q "tailscale"; then
    echo "⚠️  检测到 Tailscale VPN 接口"
    echo ""
    echo "Tailscale 特殊注意事项："
    echo "  1. 确保两台机器都在同一个 Tailscale 网络"
    echo "  2. Tailscale 默认可能阻止组播流量"
    echo "  3. 可能需要禁用组播，使用纯单播模式"
    echo ""
    echo "建议配置（禁用组播）："
    echo "export CYCLONEDDS_URI='<CycloneDDS><Domain><General><AllowMulticast>false</AllowMulticast><Interfaces><NetworkInterface name=\"$INTERFACE\" priority=\"default\" multicast=\"false\"/></Interfaces></General></Domain></CycloneDDS>'"
    echo ""
elif echo "$INTERFACE" | grep -qE "wg|vpn"; then
    echo "⚠️  检测到 VPN 接口"
    echo "VPN 可能不支持组播，建议禁用组播使用纯单播模式"
fi
echo ""

echo "========================================"
echo "故障排查总结"
echo "========================================"
echo ""
echo "常见问题和解决方案："
echo ""
echo "问题 1：防火墙阻止"
echo "  解决：sudo ./setup_firewall.sh"
echo ""
echo "问题 2：在同一台机器上测试"
echo "  解决：必须在两台不同的机器上运行发布者和订阅者"
echo ""
echo "问题 3：网络不通"
echo "  解决：使用 ping 测试网络连通性"
echo ""
echo "问题 4：VPN/Tailscale 不支持组播"
echo "  解决：禁用组播，使用单播模式"
echo ""
echo "问题 5：程序未使用正确的网络接口"
echo "  解决：确认 CYCLONEDDS_URI 环境变量已正确设置"
echo ""
echo "========================================"
echo "建议的测试步骤："
echo "========================================"
echo ""
echo "1. 在本地机器上运行订阅者："
echo "   ./run_with_network_interface.sh $INTERFACE subscriber"
echo ""
echo "2. 在远程机器上运行发布者："
if [ -n "$REMOTE_IP" ]; then
    echo "   ssh user@$REMOTE_IP"
fi
echo "   cd <项目目录>/example/lowcmd_test"
echo "   ./run_with_network_interface.sh $INTERFACE publisher"
echo ""
echo "3. 监控网络流量（可选）："
echo "   sudo tcpdump -i $INTERFACE -n 'udp portrange 7400-7419'"
echo ""
echo "4. 如果使用 Tailscale，尝试禁用组播："
if echo "$INTERFACE" | grep -q "tailscale"; then
    cat > /tmp/cyclonedds_tailscale_unicast.xml << 'EOF'
<?xml version="1.0" encoding="UTF-8"?>
<CycloneDDS xmlns="https://cdds.io/config">
  <Domain>
    <General>
      <!-- 禁用组播，使用纯单播 -->
      <AllowMulticast>false</AllowMulticast>
      <Interfaces>
        <NetworkInterface name="INTERFACE_NAME" priority="default" multicast="false"/>
      </Interfaces>
    </General>
    <Discovery>
      <!-- 单播发现配置 -->
      <ParticipantIndex>auto</ParticipantIndex>
    </Discovery>
  </Domain>
</CycloneDDS>
EOF
    sed -i "s/INTERFACE_NAME/$INTERFACE/g" /tmp/cyclonedds_tailscale_unicast.xml
    echo ""
    echo "已生成 Tailscale 单播配置：/tmp/cyclonedds_tailscale_unicast.xml"
    echo "使用方法："
    echo "  export CYCLONEDDS_URI=file:///tmp/cyclonedds_tailscale_unicast.xml"
    echo "  ../../build/bin/lowcmd_publisher"
fi
echo ""
