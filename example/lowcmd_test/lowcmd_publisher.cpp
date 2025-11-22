#include <unitree/robot/channel/channel_publisher.hpp>
#include <unitree/common/time/time_tool.hpp>
#include <unitree/idl/hg/LowCmd_.hpp>
#include <iostream>
#include <cmath>

#define TOPIC "rt/lowcmd"

using namespace unitree::robot;
using namespace unitree::common;
using namespace unitree_hg::msg::dds_;

int main(int argc, char** argv)
{
    // 解析命令行参数
    std::string network_interface = "";
    if (argc > 1)
    {
        network_interface = argv[1];
    }

    std::cout << "====================================" << std::endl;
    std::cout << "LowCmd 发布者测试程序" << std::endl;
    std::cout << "话题: " << TOPIC << std::endl;
    if (!network_interface.empty())
    {
        std::cout << "网络接口: " << network_interface << std::endl;
    }
    else
    {
        std::cout << "网络接口: 默认 (所有接口)" << std::endl;
    }
    std::cout << "====================================" << std::endl;
    std::cout << "用法: " << argv[0] << " [network_interface]" << std::endl;
    std::cout << "示例: " << argv[0] << " eth0" << std::endl;
    std::cout << "====================================" << std::endl;

    // 初始化通道工厂
    ChannelFactory::Instance()->Init(0, network_interface);
    
    // 创建发布者
    ChannelPublisher<LowCmd_> publisher(TOPIC);
    publisher.InitChannel();

    std::cout << "开始发布 LowCmd 消息..." << std::endl;
    
    int count = 0;
    while (true)
    {
        // 创建 LowCmd 消息
        LowCmd_ msg;
        
        // 设置控制模式
        msg.mode_pr(0);        // 0: PR 模式
        msg.mode_machine(5);   // 5: 29-DOF G1 型号
        
        // 设置电机指令 (示例: 设置前几个关节的目标位置)
        // 这里使用正弦波作为示例，实际应用中应该根据需求设置
        float time = count * 0.01f;  // 假设循环周期为 10ms
        
        for (size_t i = 0; i < 35; i++)
        {
            auto& motor = msg.motor_cmd()[i];
            
            if (i < 10)  // 仅控制前 10 个关节作为示例
            {
                motor.mode(10);  // 位置控制模式
                motor.q(0.1f * std::sin(time + i * 0.5f));  // 目标位置 (正弦波)
                motor.dq(0.0f);   // 目标速度
                motor.tau(0.0f);  // 前馈力矩
                motor.kp(20.0f);  // 位置增益
                motor.kd(0.5f);   // 速度增益
            }
            else
            {
                motor.mode(0);    // 其他关节设为待机模式
                motor.q(0.0f);
                motor.dq(0.0f);
                motor.tau(0.0f);
                motor.kp(0.0f);
                motor.kd(0.0f);
            }
            motor.reserve(0);
        }
        
        // 设置 CRC (实际应用中应该计算真实的 CRC 值)
        msg.crc(0);
        
        // 发布消息
        publisher.Write(msg);
        
        count++;
        if (count % 100 == 0)
        {
            std::cout << "已发布 " << count << " 条消息" << std::endl;
        }
        
        // 休眠 10ms (100Hz)
        usleep(10000);
    }

    return 0;
}
