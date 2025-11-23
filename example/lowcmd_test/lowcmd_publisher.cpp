#include <unitree/robot/channel/channel_publisher.hpp>
#include <unitree/common/time/time_tool.hpp>
#include <unitree/idl/hg/LowCmd_.hpp>
#include <iostream>
#include <iomanip>
#include <cmath>
#include <ctime>

#define TOPIC "rt/lowcmd1"

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
    std::cout << "消息频率: 100 Hz (每 10ms 一条)" << std::endl;
    std::cout << "========================================" << std::endl << std::endl;
    
    uint32_t seq_num = 0;
    int64_t start_time = GetCurrentTimeMillisecond();
    int64_t last_report_time = start_time;
    while (true)
    {
        int64_t send_time = GetCurrentTimeMillisecond();
        
        // 创建 LowCmd 消息
        LowCmd_ msg;
        
        // 设置控制模式
        msg.mode_pr(0);        // 0: PR 模式
        msg.mode_machine(5);   // 5: 29-DOF G1 型号
        
        // 设置电机指令 (示例: 设置前几个关节的目标位置)
        // 这里使用正弦波作为示例，实际应用中应该根据需求设置
        float time = seq_num * 0.01f;  // 假设循环周期为 10ms
        
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
        
        // 使用 crc 字段存储序列号（仅用于测试）
        msg.crc(seq_num);
        
        // 发布消息
        publisher.Write(msg);
        
        // 每10条打印一次发送时间
        if (seq_num % 10 == 0)
        {
            double elapsed_sec = (send_time - start_time) / 1000.0;
            time_t now_sec = send_time / 1000;
            int ms = send_time % 1000;
            struct tm* timeinfo = localtime(&now_sec);
            char time_buf[32];
            strftime(time_buf, sizeof(time_buf), "%H:%M:%S", timeinfo);
            
            std::cout << "[发送] #" << std::setw(6) << seq_num 
                      << " | 时间: " << time_buf << "." << std::setfill('0') << std::setw(3) << ms << std::setfill(' ')
                      << " | 相对: " << std::fixed << std::setprecision(3) << elapsed_sec << "s" << std::endl;
        }
        
        seq_num++;
        
        // 每秒统计一次
        int64_t current_time = GetCurrentTimeMillisecond();
        if (current_time - last_report_time >= 1000)
        {
            int64_t elapsed = current_time - start_time;
            double rate = (seq_num * 1000.0) / elapsed;
            std::cout << std::fixed << std::setprecision(1);
            std::cout << "\n[统计] 已发送: " << seq_num << " 条"
                      << " | 运行时间: " << (elapsed / 1000.0) << " s"
                      << " | 发送速率: " << rate << " Hz\n" << std::endl;
            last_report_time = current_time;
        }
        
        // 休眠 10ms (100Hz)
        usleep(10000);
    }

    return 0;
}
