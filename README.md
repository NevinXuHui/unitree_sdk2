# unitree_sdk2
Unitree robot sdk version 2.

### Prebuild environment
* OS  (Ubuntu 20.04 LTS)  
* CPU  (aarch64 and x86_64)   
* Compiler  (gcc version 9.4.0) 

### Environment Setup

Before building or running the SDK, ensure the following dependencies are installed:

- CMake (version 3.10 or higher)
- GCC (version 9.4.0)
- Make

You can install the required packages on Ubuntu 20.04 with:

```bash
apt-get update
apt-get install -y cmake g++ build-essential libyaml-cpp-dev libeigen3-dev libboost-all-dev libspdlog-dev libfmt-dev
```

### Build examples

To build the examples inside this repository:

```bash
mkdir build
cd build
cmake ..
make
```

### Installation

To build your own application with the SDK, you can install the unitree_sdk2 to your system directory:

```bash
mkdir build
cd build
cmake ..
sudo make install
```

Or install unitree_sdk2 to a specified directory:

```bash
mkdir build
cd build
cmake .. -DCMAKE_INSTALL_PREFIX=/opt/unitree_robotics
sudo make install
```

You can refer to `example/cmake_sample` on how to import the unitree_sdk2 into your CMake project. 

Note that if you install the library to other places other than `/opt/unitree_robotics`, you need to make sure the path is added to "${CMAKE_PREFIX_PATH}" so that cmake can find it with "find_package()".

### Notice
For more reference information, please go to [Unitree Document Center](https://support.unitree.com/home/zh/developer).

---

## GO2 快速使用指南

本项目已配置为仅编译 GO2 相关程序，并提供了便捷的编译和安装脚本。

### 快速编译

```bash
./build.sh
```

清理后重新编译：

```bash
./build.sh --clean
```

### 直接运行

编译完成后，可以直接运行程序：

```bash
./build/bin/go2_motion_demo eth0
```

### 安装到系统

安装到默认位置（`/opt/unitree_go2`）：

```bash
./install.sh
```

安装到自定义位置：

```bash
./install.sh -p /usr/local
```

安装后运行：

```bash
/opt/unitree_go2/bin/go2_motion_demo eth0
```

### 卸载

```bash
./uninstall.sh
```

### GO2 Motion Demo

交互式运动控制演示程序，支持：
- 动作切换：站立、趴下、坐下、恢复站立、阻尼模式
- 移动控制：前进/后退、左转/右转、左移/右移

详细使用说明请查看：[GO2_DEMO_README.md](GO2_DEMO_README.md)
