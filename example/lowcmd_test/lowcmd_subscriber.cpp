#include <unitree/robot/channel/channel_subscriber.hpp>
#include <unitree/common/time/time_tool.hpp>
#include <unitree/idl/hg/LowCmd_.hpp>
#include <iostream>
#include <iomanip>
#include <ctime>

#define TOPIC "rt/lowcmd"

using namespace unitree::robot;
using namespace unitree::common;
using namespace unitree_hg::msg::dds_;

// 统计信息
struct Statistics {
    uint32_t expected_seq = 0;      // 期望的序列号
    uint32_t received_count = 0;    // 实际接收数
    uint32_t lost_count = 0;        // 丢失数
    int64_t first_recv_time = 0;    // 首次接收时间
    int64_t last_recv_time = 0;     // 最后接收时间
    int64_t last_report_time = 0;   // 最后报告时间
} stats;

// 消息接收回调函数
void Handler(const void* msg)
{
    const LowCmd_* cmd = (const LowCmd_*)msg;
    
    int64_t recv_time = GetCurrentTimeMillisecond();
    uint32_t seq_num = cmd->crc();  // 从 crc 字段读取序列号
    
    // 初始化统计
    if (stats.received_count == 0) {
        stats.first_recv_time = recv_time;
        stats.last_report_time = recv_time;
        stats.expected_seq = seq_num;
    }
    
    // 计算丢包
    if (seq_num > stats.expected_seq) {
        uint32_t lost = seq_num - stats.expected_seq;
        stats.lost_count += lost;
    }
    stats.expected_seq = seq_num + 1;
    stats.received_count++;
    stats.last_recv_time = recv_time;
    
    // 每10条打印一次接收时间
    if (stats.received_count % 10 == 0) {
        double elapsed_sec = (recv_time - stats.first_recv_time) / 1000.0;
        time_t now_sec = recv_time / 1000;
        int ms = recv_time % 1000;
        struct tm* timeinfo = localtime(&now_sec);
        char time_buf[32];
        strftime(time_buf, sizeof(time_buf), "%H:%M:%S", timeinfo);
        
        std::cout << "[接收] #" << std::setw(6) << seq_num 
                  << " | 时间: " << time_buf << "." << std::setfill('0') << std::setw(3) << ms << std::setfill(' ')
                  << " | 相对: " << std::fixed << std::setprecision(3) << elapsed_sec << "s" << std::endl;
    }
    
    // 打印消息头信息（前 10 条）
    if (stats.received_count <= 10) {
        std::cout << "========================================" << std::endl;
        std::cout << "详细信息 - 序列号: " << seq_num 
                  << " | 接收时间: " << recv_time << " ms" << std::endl;
        std::cout << "控制模式:" << std::endl;
        std::cout << "  mode_pr: " << (int)cmd->mode_pr() << " (0:PR, 1:AB)" << std::endl;
        std::cout << "  mode_machine: " << (int)cmd->mode_machine() 
                  << " (4:23-DOF, 5:29-DOF, 6:27-DOF)" << std::endl;
        
        // 打印前 5 个电机的控制指令
        std::cout << std::endl << "前 5 个电机指令:" << std::endl;
        std::cout << std::fixed << std::setprecision(3);
        std::cout << "ID | Mode | Position | Velocity |  Torque  |   Kp   |   Kd   " << std::endl;
        std::cout << "---+------+----------+----------+----------+--------+--------" << std::endl;
        
        for (size_t i = 0; i < 5 && i < 35; i++)
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
    
    // 每秒统计一次
    if (recv_time - stats.last_report_time >= 1000) {
        int64_t elapsed = recv_time - stats.first_recv_time;
        double recv_rate = (stats.received_count * 1000.0) / elapsed;
        double loss_rate = (stats.lost_count * 100.0) / (stats.received_count + stats.lost_count);
        
        std::cout << std::fixed << std::setprecision(2);
        std::cout << "\n[统计] 已接收: " << stats.received_count << " 条"
                  << " | 丢失: " << stats.lost_count << " 条"
                  << " | 丢包率: " << loss_rate << "%"
                  << " | 接收速率: " << recv_rate << " Hz\n" << std::endl;
        
        stats.last_report_time = recv_time;
    }
}

int main(int argc, char** argv)
{
    // 解析命令行参数
    std::string network_interface = "";
    if (argc > 1)
    {
        network_interface = argv[1];
    }

    std::cout << "====================================" << std::endl;
    std::cout << "LowCmd 订阅者测试程序" << std::endl;
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
    std::cout << "等待消息..." << std::endl << std::endl;

    // 初始化通道工厂
    ChannelFactory::Instance()->Init(0, network_interface);
    
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
