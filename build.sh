#!/bin/bash

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="${SCRIPT_DIR}/build"
BUILD_TYPE="Release"
CLEAN_BUILD=0
JOBS=$(nproc)

print_usage() {
    cat << EOF
Usage: $0 [OPTIONS]

编译 unitree_sdk2 (GO2机器人)

OPTIONS:
    -h, --help          显示帮助信息
    -c, --clean         清理构建目录后重新编译
    -d, --debug         使用 Debug 模式编译 (默认: Release)
    -j, --jobs N        并行编译任务数 (默认: $(nproc))

示例:
    $0                  # 编译 go2 示例程序
    $0 --clean          # 清理后重新编译
    $0 --debug          # Debug模式编译

EOF
}

while [[ $# -gt 0 ]]; do
    case $1 in
        -h|--help)
            print_usage
            exit 0
            ;;
        -c|--clean)
            CLEAN_BUILD=1
            shift
            ;;
        -d|--debug)
            BUILD_TYPE="Debug"
            shift
            ;;
        -j|--jobs)
            JOBS="$2"
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
echo "unitree_sdk2 编译脚本 (GO2)"
echo "========================================"
echo "构建目录: ${BUILD_DIR}"
echo "构建类型: ${BUILD_TYPE}"
echo "并行任务: ${JOBS}"
echo "========================================"

check_dependencies() {
    echo "检查依赖..."
    local missing_deps=()
    
    if ! command -v cmake &> /dev/null; then
        missing_deps+=("cmake")
    fi
    
    if ! command -v g++ &> /dev/null; then
        missing_deps+=("g++")
    fi
    
    if ! command -v make &> /dev/null; then
        missing_deps+=("make")
    fi
    
    if [ ${#missing_deps[@]} -ne 0 ]; then
        echo "错误: 缺少以下依赖: ${missing_deps[*]}"
        echo ""
        echo "请运行以下命令安装依赖:"
        echo "sudo apt-get update"
        echo "sudo apt-get install -y cmake g++ build-essential libyaml-cpp-dev libeigen3-dev libboost-all-dev libspdlog-dev libfmt-dev"
        exit 1
    fi
    
    echo "依赖检查通过!"
}

check_dependencies

if [ ${CLEAN_BUILD} -eq 1 ] && [ -d "${BUILD_DIR}" ]; then
    echo "清理构建目录: ${BUILD_DIR}"
    rm -rf "${BUILD_DIR}"
fi

if [ ! -d "${BUILD_DIR}" ]; then
    echo "创建构建目录: ${BUILD_DIR}"
    mkdir -p "${BUILD_DIR}"
fi

cd "${BUILD_DIR}"

echo ""
echo "运行 CMake..."
cmake -DCMAKE_BUILD_TYPE=${BUILD_TYPE} -DBUILD_EXAMPLES=ON ..

echo ""
echo "开始编译..."
make -j${JOBS}

echo ""
echo "========================================"
echo "编译成功!"
echo "========================================"
echo "示例程序位于: ${BUILD_DIR}/bin"
echo ""
echo "【GO2 机器人示例】"
echo "  - go2_motion_demo (推荐: 交互式移动和动作控制)"
echo "  - go2_sport_client"
echo "  - go2_low_level"
echo "  - go2_stand_example"
echo "  - go2_robot_state_client"
echo "  - go2_video_client"
echo "  - go2_vui_client"
echo ""
echo "【测试工具】"
echo "  - lowcmd_publisher (发布 rt/lowcmd 话题)"
echo "  - lowcmd_subscriber (订阅 rt/lowcmd 话题)"
echo ""
echo "运行方式:"
echo "  GO2 示例 - 直接运行:"
echo "    ./build/bin/go2_motion_demo eth0"
echo ""
echo "  LowCmd 测试 - 使用测试脚本:"
echo "    ./example/lowcmd_test/run_test.sh"
echo ""
echo "  或手动运行:"
echo "    ./build/bin/lowcmd_subscriber  # 终端1"
echo "    ./build/bin/lowcmd_publisher   # 终端2"
echo ""
echo "  系统安装:"
echo "    ./install.sh"
echo "    /opt/unitree_go2/bin/go2_motion_demo eth0"
echo ""
echo "完成!"
