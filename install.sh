#!/bin/bash

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ARCH=$(uname -m)
BUILD_DIR="${SCRIPT_DIR}/build"
INSTALL_PREFIX="/opt/unitree_go2"

print_usage() {
    cat << EOF
用法: $0 [选项]

将 GO2 程序安装到系统目录

选项:
    -h, --help          显示帮助信息
    -p, --prefix PATH   指定安装路径 (默认: /opt/unitree_go2)

示例:
    $0                  # 安装到默认位置 /opt/unitree_go2
    $0 -p /usr/local    # 安装到 /usr/local

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
echo "GO2 程序安装脚本"
echo "========================================"
echo "系统架构: ${ARCH}"

# 显示构建目录信息
if [ -L "${BUILD_DIR}" ]; then
    BUILD_TARGET=$(readlink "${BUILD_DIR}")
    echo "构建目录: ${BUILD_DIR} -> ${BUILD_TARGET}"
else
    echo "构建目录: ${BUILD_DIR}"
fi

echo "安装路径: ${INSTALL_PREFIX}"
echo "========================================"

# Check if build directory exists
if [ ! -d "${BUILD_DIR}" ]; then
    echo "错误: 构建目录不存在: ${BUILD_DIR}"
    echo "请先运行 ./build.sh 编译程序"
    exit 1
fi

# Check if bin directory exists
if [ ! -d "${BUILD_DIR}/bin" ]; then
    echo "错误: 可执行文件目录不存在: ${BUILD_DIR}/bin"
    echo "请先运行 ./build.sh 编译程序"
    exit 1
fi

# Check if there are any executables
if [ -z "$(ls -A ${BUILD_DIR}/bin/go2_* 2>/dev/null)" ]; then
    echo "错误: 未找到 GO2 可执行文件"
    echo "请先运行 ./build.sh 编译程序"
    exit 1
fi

# Create installation directories
echo ""
echo "创建安装目录..."
sudo mkdir -p "${INSTALL_PREFIX}/bin"
sudo mkdir -p "${INSTALL_PREFIX}/doc"

# Install executables
echo ""
echo "安装可执行文件..."
for file in ${BUILD_DIR}/bin/go2_*; do
    if [ -f "$file" ]; then
        filename=$(basename "$file")
        echo "  安装: $filename"
        sudo cp "$file" "${INSTALL_PREFIX}/bin/"
        sudo chmod 755 "${INSTALL_PREFIX}/bin/$filename"
    fi
done

# Install documentation
echo ""
echo "安装文档..."
if [ -f "${SCRIPT_DIR}/GO2_DEMO_README.md" ]; then
    sudo cp "${SCRIPT_DIR}/GO2_DEMO_README.md" "${INSTALL_PREFIX}/doc/"
    echo "  已安装: GO2_DEMO_README.md"
fi

if [ -f "${SCRIPT_DIR}/README.md" ]; then
    sudo cp "${SCRIPT_DIR}/README.md" "${INSTALL_PREFIX}/doc/"
    echo "  已安装: README.md"
fi

# Create symbolic links in /usr/local/bin (optional)
echo ""
read -p "是否在 /usr/local/bin 中创建符号链接? (y/N): " CREATE_LINKS
if [[ "$CREATE_LINKS" =~ ^[Yy]$ ]]; then
    echo "创建符号链接..."
    for file in ${INSTALL_PREFIX}/bin/go2_*; do
        if [ -f "$file" ]; then
            filename=$(basename "$file")
            if [ -L "/usr/local/bin/$filename" ] || [ -f "/usr/local/bin/$filename" ]; then
                sudo rm -f "/usr/local/bin/$filename"
            fi
            sudo ln -s "$file" "/usr/local/bin/$filename"
            echo "  链接: /usr/local/bin/$filename -> $file"
        fi
    done
fi

echo ""
echo "========================================"
echo "安装完成!"
echo "========================================"
echo "安装位置: ${INSTALL_PREFIX}"
echo ""
echo "已安装的程序:"
ls -1 ${INSTALL_PREFIX}/bin/
echo ""
echo "使用方法:"
echo "  ${INSTALL_PREFIX}/bin/go2_motion_demo eth0"
echo ""
if [[ "$CREATE_LINKS" =~ ^[Yy]$ ]]; then
    echo "或者直接运行 (已创建符号链接):"
    echo "  go2_motion_demo eth0"
    echo ""
fi
echo "文档位置: ${INSTALL_PREFIX}/doc/"
echo "========================================"
