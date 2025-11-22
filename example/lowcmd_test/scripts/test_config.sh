#!/bin/bash
# 测试 DDS 配置是否正确

echo "======================================"
echo "DDS 配置测试工具"
echo "======================================"
echo ""

# 检查是否提供网络接口参数
if [ $# -eq 0 ]; then
    echo "用法: $0 <网络接口名>"
    echo ""
    echo "示例:"
    echo "  $0 eth0"
    echo "  $0 eth10"
    echo ""
    echo "可用的网络接口："
    ip addr show | grep -E "^[0-9]+:" | awk '{print "  - " $2}' | sed 's/:$//'
    echo ""
    exit 1
fi

INTERFACE=$1

# 检查网络接口是否存在
if ! ip addr show "$INTERFACE" &> /dev/null; then
    echo "❌ 错误: 网络接口 '$INTERFACE' 不存在！"
    echo ""
    echo "可用的网络接口："
    ip addr show | grep -E "^[0-9]+:" | awk '{print "  - " $2}' | sed 's/:$//'
    exit 1
fi

echo "✅ 网络接口 $INTERFACE 存在"
echo ""

# 显示接口信息
echo "接口信息："
ip addr show "$INTERFACE" | grep "inet " | awk '{print "  IP 地址: " $2}'
echo ""

# 测试标准环境变量格式
echo "测试标准环境变量格式："
echo "======================================"
TEST_URI='<CycloneDDS><Domain><General><AllowMulticast>spdp</AllowMulticast><Interfaces><NetworkInterface name="'$INTERFACE'" priority="default" multicast="default"/></Interfaces></General></Domain></CycloneDDS>'
echo "$TEST_URI"
echo ""

# 检查 XML 语法
if echo "$TEST_URI" | grep -q "<CycloneDDS>.*</CycloneDDS>"; then
    echo "✅ XML 格式正确"
else
    echo "❌ XML 格式错误"
    exit 1
fi

# 检查必要的配置项
echo ""
echo "检查配置项："
if echo "$TEST_URI" | grep -q "<AllowMulticast>spdp</AllowMulticast>"; then
    echo "  ✅ AllowMulticast=spdp"
else
    echo "  ❌ 缺少 AllowMulticast 配置"
fi

if echo "$TEST_URI" | grep -q "NetworkInterface.*name=\"$INTERFACE\""; then
    echo "  ✅ NetworkInterface name=\"$INTERFACE\""
else
    echo "  ❌ NetworkInterface 配置错误"
fi

if echo "$TEST_URI" | grep -q 'priority="default"'; then
    echo "  ✅ priority=\"default\""
else
    echo "  ⚠️  缺少 priority 属性（可选）"
fi

if echo "$TEST_URI" | grep -q 'multicast="default"'; then
    echo "  ✅ multicast=\"default\""
else
    echo "  ⚠️  multicast 属性设置不同"
fi

echo ""
echo "======================================"
echo "配置测试完成！"
echo "======================================"
echo ""
echo "使用此配置运行程序："
echo ""
ARCH=$(uname -m)
echo "export CYCLONEDDS_URI='$TEST_URI'"
echo "../../build_${ARCH}/bin/lowcmd_publisher"
echo ""
echo "或者使用便捷脚本："
echo ""
echo "  ./run_with_network_interface.sh $INTERFACE publisher"
echo "  ./run_with_network_interface.sh $INTERFACE subscriber"
echo ""
