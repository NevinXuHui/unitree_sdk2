#include <unitree/robot/channel/channel_subscriber.hpp>
#include <unitree/common/time/time_tool.hpp>
#include <unitree/idl/hg/LowCmd_.hpp>
#include <iostream>
#include <iomanip>

#define TOPIC "rt/lowcmd"

using namespace unitree::robot;
using namespace unitree::common;
using namespace unitree_hg::msg::dds_;

// 消息接收回调函数
void Handler(const void* msg)
{
    const LowCmd_* cmd = (const LowCmd_*)msg;
    
    static int msg_count = 0;
    msg_count++;
    
    // 打印消息头信息
    std::cout << "========================================" << std::endl;
    std::cout << "接收到 LowCmd 消息 #" << msg_count << std::endl;
    std::cout << "时间戳: " << GetCurrentTimeMillisecond() << " ms" << std::endl;
    std::cout << "控制模式:" << std::endl;
    std::cout << "  mode_pr: " << (int)cmd->mode_pr() << " (0:PR, 1:AB)" << std::endl;
    std::cout << "  mode_machine: " << (int)cmd->mode_machine() 
              << " (4:23-DOF, 5:29-DOF, 6:27-DOF)" << std::endl;
    std::cout << "  CRC: " << cmd->crc() << std::endl;
    
    // 打印前 10 个电机的控制指令
    std::cout << std::endl << "前 10 个电机指令:" << std::endl;
    std::cout << std::fixed << std::setprecision(3);
    std::cout << "ID | Mode | Position | Velocity |  Torque  |   Kp   |   Kd   " << std::endl;
    std::cout << "---+------+----------+----------+----------+--------+--------" << std::endl;
    
    for (size_t i = 0; i < 10 && i < 35; i++)
    {
        const auto& motor = cmd->motor_cmd()[i];
        std::cout << std::setw(2) << i << " | "
                  << std::setw(4) << (int)motor.mode() << " | "
                  << std::setw(8) << motor.q() << " | "
                  << std::setw(8) << motor.dq() << " | "
                  << std::setw(8) << motor.tau() << " | "
                  << std::setw(6) << motor.kp() << " | "
                  << std::setw(6) << motor.kd() << std::endl;
    }
    
    // 统计非零模式的电机数量
    int active_motors = 0;
    for (size_t i = 0; i < 35; i++)
    {
        if (cmd->motor_cmd()[i].mode() != 0)
        {
            active_motors++;
        }
    }
    std::cout << std::endl << "激活的电机数量: " << active_motors << " / 35" << std::endl;
    std::cout << "========================================" << std::endl << std::endl;
}

int main()
{
    std::cout << "====================================" << std::endl;
    std::cout << "LowCmd 订阅者测试程序" << std::endl;
    std::cout << "话题: " << TOPIC << std::endl;
    std::cout << "====================================" << std::endl;
    std::cout << "等待消息..." << std::endl << std::endl;

    // 初始化通道工厂
    ChannelFactory::Instance()->Init(0);
    
    // 创建订阅者并注册回调函数
    ChannelSubscriber<LowCmd_> subscriber(TOPIC);
    subscriber.InitChannel(Handler);

    std::cout << "订阅者已启动，按 Ctrl+C 退出" << std::endl << std::endl;

    // 持续运行
    while (true)
    {
        sleep(10);
    }

    return 0;
}
