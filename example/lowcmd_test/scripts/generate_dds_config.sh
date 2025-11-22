#!/bin/bash
# 生成自定义网络接口的 CycloneDDS 配置

if [ $# -eq 0 ]; then
    echo "======================================"
    echo "CycloneDDS 配置生成器"
    echo "======================================"
    echo ""
    echo "用法: $0 <网络接口名>"
    echo ""
    echo "示例:"
    echo "  $0 eth0      - 生成使用 eth0 的配置"
    echo "  $0 eth10     - 生成使用 eth10 的配置"
    echo "  $0 wlan0     - 生成使用 wlan0 的配置"
    echo ""
    echo "可用的网络接口："
    ip addr show | grep -E "^[0-9]+:" | awk '{print "  - " $2}' | sed 's/:$//'
    echo ""
    exit 1
fi

INTERFACE=$1
CONFIG_FILE="cyclonedds_${INTERFACE}.xml"
SCRIPT_FILE="run_with_${INTERFACE}.sh"

# 检查接口是否存在
if ! ip addr show "$INTERFACE" &> /dev/null; then
    echo "❌ 错误: 网络接口 '$INTERFACE' 不存在！"
    echo ""
    echo "可用的网络接口："
    ip addr show | grep -E "^[0-9]+:" | awk '{print "  - " $2}' | sed 's/:$//'
    exit 1
fi

echo "======================================"
echo "生成 $INTERFACE 的 DDS 配置"
echo "======================================"

# 生成 XML 配置文件
cat > "$CONFIG_FILE" << EOF
<?xml version="1.0" encoding="UTF-8"?>
<CycloneDDS xmlns="https://cdds.io/config" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xsi:schemaLocation="https://cdds.io/config https://raw.githubusercontent.com/eclipse-cyclonedds/cyclonedds/master/etc/cyclonedds.xsd">
  <Domain>
    <General>
      <!-- 网络接口配置 -->
      <Interfaces>
        <NetworkInterface name="$INTERFACE" priority="default" multicast="default"/>
      </Interfaces>
      
      <!-- 组播配置 - 仅用于 SPDP (服务发现) -->
      <AllowMulticast>spdp</AllowMulticast>
    </General>
    
    <!-- 可选：启用追踪日志用于调试 -->
    <!--
    <Tracing>
      <Verbosity>finest</Verbosity>
      <OutputFile>dds_${INTERFACE}.log</OutputFile>
    </Tracing>
    -->
  </Domain>
</CycloneDDS>
EOF

echo "✓ 已生成配置文件: $CONFIG_FILE"

# 生成运行脚本
cat > "$SCRIPT_FILE" << 'SCRIPT_EOF'
#!/bin/bash
# 使用 INTERFACE_PLACEHOLDER 网络接口运行 DDS 程序

# 方法 1: 使用环境变量（内联 XML）
# AllowMulticast=spdp: 仅在 SPDP（服务发现）中使用组播，数据传输使用单播
export CYCLONEDDS_URI='<CycloneDDS><Domain><General><AllowMulticast>spdp</AllowMulticast><Interfaces><NetworkInterface name="INTERFACE_PLACEHOLDER" priority="default" multicast="default"/></Interfaces></General></Domain></CycloneDDS>'

# 方法 2: 使用配置文件（注释掉方法 1，启用方法 2）
# export CYCLONEDDS_URI=file://$PWD/CONFIG_FILE_PLACEHOLDER

echo "======================================"
echo "使用 INTERFACE_PLACEHOLDER 网络接口运行 DDS 程序"
echo "======================================"
echo "网络接口: INTERFACE_PLACEHOLDER"
echo "组播模式: SPDP"
echo ""

# 检查接口是否存在
if ! ip addr show INTERFACE_PLACEHOLDER &> /dev/null; then
    echo "⚠️  警告: 网络接口 INTERFACE_PLACEHOLDER 不存在！"
    echo ""
    echo "可用的网络接口："
    ip addr show | grep -E "^[0-9]+:" | awk '{print "  - " $2}' | sed 's/:$//'
    echo ""
    exit 1
fi

echo "✓ 网络接口 INTERFACE_PLACEHOLDER 已找到"
echo ""
echo "INTERFACE_PLACEHOLDER IP 地址："
ip addr show INTERFACE_PLACEHOLDER | grep "inet " | awk '{print "  " $2}'
echo ""

# 检查命令行参数
if [ $# -eq 0 ]; then
    echo "用法: $0 <publisher|subscriber>"
    echo ""
    echo "示例："
    echo "  $0 publisher   - 运行发布者"
    echo "  $0 subscriber  - 运行订阅者"
    echo "======================================"
    exit 1
fi

# 根据参数运行相应程序
case "$1" in
    publisher)
        echo "启动发布者..."
        echo "======================================"
        ../../build/bin/lowcmd_publisher
        ;;
    subscriber)
        echo "启动订阅者..."
        echo "======================================"
        ../../build/bin/lowcmd_subscriber
        ;;
    *)
        echo "❌ 无效参数: $1"
        echo "请使用 'publisher' 或 'subscriber'"
        exit 1
        ;;
esac
SCRIPT_EOF

# 替换占位符
sed -i "s/INTERFACE_PLACEHOLDER/$INTERFACE/g" "$SCRIPT_FILE"
sed -i "s/CONFIG_FILE_PLACEHOLDER/$CONFIG_FILE/g" "$SCRIPT_FILE"

chmod +x "$SCRIPT_FILE"

echo "✓ 已生成运行脚本: $SCRIPT_FILE"
echo ""
echo "======================================"
echo "使用方法："
echo "======================================"
echo ""
echo "方法 1: 使用运行脚本（推荐）"
echo "  # 终端 1: 运行订阅者"
echo "  ./$SCRIPT_FILE subscriber"
echo ""
echo "  # 终端 2: 运行发布者"
echo "  ./$SCRIPT_FILE publisher"
echo ""
echo "方法 2: 使用环境变量（内联 XML）"
echo "  export CYCLONEDDS_URI='<CycloneDDS><Domain><General><AllowMulticast>spdp</AllowMulticast><Interfaces><NetworkInterface name=\"$INTERFACE\" priority=\"default\" multicast=\"default\"/></Interfaces></General></Domain></CycloneDDS>'"
echo "  ../../build/bin/lowcmd_publisher"
echo ""
echo "方法 3: 使用配置文件"
echo "  export CYCLONEDDS_URI=file://\$PWD/$CONFIG_FILE"
echo "  ../../build/bin/lowcmd_publisher"
echo ""
echo "======================================"
