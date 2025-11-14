#!/bin/bash

set -e

INSTALL_PREFIX="/opt/unitree_go2"

print_usage() {
    cat << EOF
用法: $0 [选项]

卸载 GO2 程序

选项:
    -h, --help          显示帮助信息
    -p, --prefix PATH   指定安装路径 (默认: /opt/unitree_go2)

示例:
    $0                  # 从默认位置卸载
    $0 -p /usr/local    # 从指定位置卸载

EOF
}

while [[ $# -gt 0 ]]; do
    case $1 in
        -h|--help)
            print_usage
            exit 0
            ;;
        -p|--prefix)
            INSTALL_PREFIX="$2"
            shift 2
            ;;
        *)
            echo "未知选项: $1"
            print_usage
            exit 1
            ;;
    esac
done

echo "========================================"
echo "GO2 程序卸载脚本"
echo "========================================"
echo "安装路径: ${INSTALL_PREFIX}"
echo "========================================"

# Check if installation directory exists
if [ ! -d "${INSTALL_PREFIX}" ]; then
    echo "警告: 安装目录不存在: ${INSTALL_PREFIX}"
    echo "程序可能未安装或已被删除"
    exit 0
fi

echo ""
echo "将要删除以下内容:"
if [ -d "${INSTALL_PREFIX}/bin" ]; then
    echo ""
    echo "可执行文件:"
    ls -1 ${INSTALL_PREFIX}/bin/ 2>/dev/null || echo "  (无)"
fi

if [ -d "${INSTALL_PREFIX}/doc" ]; then
    echo ""
    echo "文档文件:"
    ls -1 ${INSTALL_PREFIX}/doc/ 2>/dev/null || echo "  (无)"
fi

echo ""
read -p "确认卸载? (y/N): " CONFIRM
if [[ ! "$CONFIRM" =~ ^[Yy]$ ]]; then
    echo "取消卸载"
    exit 0
fi

# Remove symbolic links from /usr/local/bin
echo ""
echo "检查并删除符号链接..."
for file in ${INSTALL_PREFIX}/bin/go2_* 2>/dev/null; do
    if [ -f "$file" ]; then
        filename=$(basename "$file")
        if [ -L "/usr/local/bin/$filename" ]; then
            echo "  删除链接: /usr/local/bin/$filename"
            sudo rm -f "/usr/local/bin/$filename"
        fi
    fi
done

# Remove installation directory
echo ""
echo "删除安装目录..."
sudo rm -rf "${INSTALL_PREFIX}"

echo ""
echo "========================================"
echo "卸载完成!"
echo "========================================"
