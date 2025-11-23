#include <unitree/robot/channel/channel_subscriber.hpp>
#include <unitree/common/time/time_tool.hpp>
#include <unitree/idl/hg/LowCmd_.hpp>
#include <iostream>
#include <iomanip>
#include <ctime>
#include <cstdio>

#define TOPIC "rt/lowcmd"

// ANSI 颜色代码
#define COLOR_RESET   "\033[0m"
#define COLOR_RED     "\033[1;31m"
#define COLOR_GREEN   "\033[1;32m"
#define COLOR_YELLOW  "\033[1;33m"
#define COLOR_BLUE    "\033[1;34m"
#define COLOR_CYAN    "\033[1;36m"

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
        stats.last_recv_time = recv_time;
    }
    
    // 检测接收时间间隔
    int64_t time_interval = recv_time - stats.last_recv_time;
    
    // 如果时间间隔大于30ms，输出详细日志
    if (time_interval > 30) {
        time_t now_sec = recv_time / 1000;
        int ms = recv_time % 1000;
        struct tm* timeinfo = localtime(&now_sec);
        char time_buf[32];
        strftime(time_buf, sizeof(time_buf), "%H:%M:%S", timeinfo);
        
        printf("%s\n[时间间隔警告] 时间: %s.%03d | 时间间隔: %ld ms | 序列号: %u\n%s\n",
               COLOR_RED, time_buf, ms, time_interval, seq_num, COLOR_RESET);
    }
    
    // // 计算丢包（处理序列号回绕）
    // if (seq_num != stats.expected_seq) {
    //     int64_t diff = (int64_t)seq_num - (int64_t)stats.expected_seq;
    //     // 只有在差值合理的情况下才计入丢包（避免序列号回绕导致的异常值）
    //     // 假设合理的丢包范围是 1 到 100000
    //     if (diff > 0 && diff < 100000) {
    //         stats.lost_count += diff;
    //     }
    // }
    
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
        
        printf("%s[接收] #%6u | 时间: %s.%03d | 相对: %.3fs%s\n",
               COLOR_GREEN, seq_num, time_buf, ms, elapsed_sec, COLOR_RESET);
    }
    
    // 打印消息头信息（前 10 条）
    if (stats.received_count <= 10) {
        std::cout << COLOR_CYAN << "========================================" << std::endl;
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
        std::cout << "========================================" << COLOR_RESET << std::endl << std::endl;
    }
    
    // 每秒统计一次
    if (recv_time - stats.last_report_time >= 1000) {
        int64_t elapsed = recv_time - stats.first_recv_time;
        double recv_rate = (stats.received_count * 1000.0) / elapsed;
        double loss_rate = (stats.lost_count * 100.0) / (stats.received_count + stats.lost_count);
        
        // 根据丢包率选择颜色
        const char* color;
        if (loss_rate == 0.0) {
            color = COLOR_GREEN;  // 无丢包：绿色
        } else if (loss_rate < 1.0) {
            color = COLOR_CYAN;   // 丢包率 < 1%：青色
        } else if (loss_rate < 5.0) {
            color = COLOR_YELLOW; // 丢包率 < 5%：黄色
        } else {
            color = COLOR_RED;    // 丢包率 >= 5%：红色
        }
        
        printf("%s\n[统计] 已接收: %u 条 | 丢失: %u 条 | 丢包率: %.2f%% | 接收速率: %.2f Hz\n%s\n",
               color, stats.received_count, stats.lost_count, loss_rate, recv_rate, COLOR_RESET);
        
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
