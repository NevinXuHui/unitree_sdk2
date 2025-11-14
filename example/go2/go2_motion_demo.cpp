/**********************************************************************
 GO2 运动控制演示程序 - 交互式移动和动作切换控制
***********************************************************************/

#include <iostream>
#include <thread>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <unitree/robot/go2/sport/sport_client.hpp>
#include <unitree/robot/channel/channel_subscriber.hpp>
#include <unitree/idl/go2/SportModeState_.hpp>

#define TOPIC_HIGHSTATE "rt/sportmodestate"

using namespace unitree::common;

enum ControlMode
{
  MODE_IDLE = 0,
  MODE_STAND,
  MODE_STAND_DOWN,
  MODE_BALANCE_STAND,
  MODE_WALK,
  MODE_SIT,
  MODE_DAMPING,
  MODE_RECOVERY,
  // Special actions
  MODE_HELLO,
  MODE_STRETCH,
  MODE_CONTENT,
  MODE_HEART,
  MODE_SCRAPE,
  MODE_POSE,
  // Dance
  MODE_DANCE1,
  MODE_DANCE2,
  // Jump
  MODE_FRONT_JUMP,
  MODE_FRONT_POUNCE,
  // Flip (dangerous)
  MODE_FRONT_FLIP,
  MODE_BACK_FLIP,
  MODE_LEFT_FLIP,
  // Gait modes
  MODE_FREE_WALK,
  MODE_CLASSIC_WALK,
  MODE_TROT_RUN,
  MODE_STATIC_WALK,
  MODE_ECONOMIC_GAIT,
  MODE_FREE_BOUND,
  MODE_FREE_JUMP,
  MODE_FREE_AVOID,
  MODE_HAND_STAND,
  MODE_WALK_UPRIGHT,
  MODE_CROSS_STEP
};

class GO2MotionDemo
{
public:
  enum MenuMode
  {
    MENU_MAIN = 0,
    MENU_ACTIONS,
    MENU_GAIT
  };

  GO2MotionDemo()
  {
    current_mode = MODE_IDLE;
    menu_mode = MENU_MAIN;
    vx = 0.0;
    vy = 0.0;
    vyaw = 0.0;
    action_timer = 0.0;
    action_duration = 0.0;
    pose_enabled = false;
    handstand_enabled = false;
    last_move_key = 0;
    key_press_time = 0.0;
    move_stop_timer = 0.0;
    
    sport_client.SetTimeout(10.0f);
    sport_client.Init();

    suber.reset(new unitree::robot::ChannelSubscriber<unitree_go::msg::dds_::SportModeState_>(TOPIC_HIGHSTATE));
    suber->InitChannel(std::bind(&GO2MotionDemo::HighStateHandler, this, std::placeholders::_1), 1);
  }

  void PrintMenu()
  {
    std::cout << "\n========================================" << std::endl;
    std::cout << "    GO2 运动控制演示程序" << std::endl;
    std::cout << "========================================" << std::endl;
    
    if (menu_mode == MENU_MAIN)
    {
      std::cout << "【主菜单】" << std::endl;
      std::cout << "\n基础动作:" << std::endl;
      std::cout << "  1 - 站立" << std::endl;
      std::cout << "  2 - 趴下" << std::endl;
      std::cout << "  3 - 坐下" << std::endl;
      std::cout << "  4 - 平衡站立" << std::endl;
      std::cout << "  5 - 恢复站立" << std::endl;
      std::cout << "  6 - 阻尼模式" << std::endl;
      std::cout << "  0 - 停止/待机" << std::endl;
      std::cout << "\n移动控制（长按持续移动，松开自动停止）:" << std::endl;
      std::cout << "     W" << std::endl;
      std::cout << "  A  S  D - 前进/左移/后退/右移" << std::endl;
      std::cout << "  Q/E - 左转/右转  X - 立即停止" << std::endl;
      std::cout << "  ←/→ - 左转/右转（方向键）" << std::endl;
      std::cout << "\n菜单切换:" << std::endl;
      std::cout << "  m - 切换到【动作菜单】" << std::endl;
      std::cout << "  g - 切换到【步态菜单】" << std::endl;
      std::cout << "  h - 显示帮助  ESC - 退出" << std::endl;
    }
    else if (menu_mode == MENU_ACTIONS)
    {
      std::cout << "【动作菜单】" << std::endl;
      std::cout << "\n表情动作:" << std::endl;
      std::cout << "  1 - 打招呼   2 - 伸懒腰   3 - 开心" << std::endl;
      std::cout << "  4 - 比心     5 - 拜年     6 - 摆姿势" << std::endl;
      std::cout << "\n舞蹈:" << std::endl;
      std::cout << "  7 - 舞蹈1    8 - 舞蹈2" << std::endl;
      std::cout << "\n跳跃:" << std::endl;
      std::cout << "  9 - 前跳     p - 扑人" << std::endl;
      std::cout << "\n⚠️  高难度动作（危险）:" << std::endl;
      std::cout << "  f - 前空翻   b - 后空翻   l - 左空翻" << std::endl;
      std::cout << "\n返回:" << std::endl;
      std::cout << "  m - 返回【主菜单】  0 - 停止动作" << std::endl;
    }
    else if (menu_mode == MENU_GAIT)
    {
      std::cout << "【步态菜单】" << std::endl;
      std::cout << "\nAI 步态:" << std::endl;
      std::cout << "  1 - 灵动模式（默认）  2 - 经典步态" << std::endl;
      std::cout << "\n常规步态:" << std::endl;
      std::cout << "  3 - 跑步模式          4 - 行走模式" << std::endl;
      std::cout << "  5 - 续航模式" << std::endl;
      std::cout << "\n特殊步态:" << std::endl;
      std::cout << "  6 - 并腿跑   7 - 跳跃跑   8 - 闪避模式" << std::endl;
      std::cout << "\n⚠️  高级步态（电机易过热）:" << std::endl;
      std::cout << "  9 - 倒立行走   u - 直立行走   c - 交叉步" << std::endl;
      std::cout << "\n返回:" << std::endl;
      std::cout << "  g - 返回【主菜单】  0 - 停止" << std::endl;
    }
    
    std::cout << "========================================" << std::endl;
    std::cout << "当前菜单: " << GetMenuName() << std::endl;
    std::cout << "当前模式: " << GetModeName() << std::endl;
    std::cout << "当前速度: vx=" << vx << " vy=" << vy << " vyaw=" << vyaw << std::endl;
    std::cout << "========================================" << std::endl;
  }

  std::string GetMenuName()
  {
    switch(menu_mode)
    {
      case MENU_MAIN: return "主菜单";
      case MENU_ACTIONS: return "动作菜单";
      case MENU_GAIT: return "步态菜单";
      default: return "未知";
    }
  }

  std::string GetModeName()
  {
    switch(current_mode)
    {
      case MODE_IDLE: return "待机";
      case MODE_STAND: return "站立";
      case MODE_STAND_DOWN: return "趴下中";
      case MODE_BALANCE_STAND: return "平衡站立";
      case MODE_WALK: return "行走";
      case MODE_SIT: return "坐下";
      case MODE_DAMPING: return "阻尼模式";
      case MODE_RECOVERY: return "恢复站立";
      case MODE_HELLO: return "打招呼";
      case MODE_STRETCH: return "伸懒腰";
      case MODE_CONTENT: return "开心";
      case MODE_HEART: return "比心";
      case MODE_SCRAPE: return "拜年";
      case MODE_POSE: return "摆姿势";
      case MODE_DANCE1: return "舞蹈1";
      case MODE_DANCE2: return "舞蹈2";
      case MODE_FRONT_JUMP: return "前跳";
      case MODE_FRONT_POUNCE: return "扑人";
      case MODE_FRONT_FLIP: return "前空翻";
      case MODE_BACK_FLIP: return "后空翻";
      case MODE_LEFT_FLIP: return "左空翻";
      case MODE_FREE_WALK: return "灵动模式";
      case MODE_CLASSIC_WALK: return "经典步态";
      case MODE_TROT_RUN: return "跑步模式";
      case MODE_STATIC_WALK: return "行走模式";
      case MODE_ECONOMIC_GAIT: return "续航模式";
      case MODE_FREE_BOUND: return "并腿跑";
      case MODE_FREE_JUMP: return "跳跃跑";
      case MODE_FREE_AVOID: return "闪避模式";
      case MODE_HAND_STAND: return "倒立行走";
      case MODE_WALK_UPRIGHT: return "直立行走";
      case MODE_CROSS_STEP: return "交叉步";
      default: return "未知";
    }
  }

  void RobotControl()
  {
    // Update action timer
    if (action_duration > 0.0)
    {
      action_timer += dt;
      if (action_timer >= action_duration)
      {
        current_mode = MODE_IDLE;
        action_timer = 0.0;
        action_duration = 0.0;
        std::cout << ">> 动作完成，切换到待机模式" << std::endl;
      }
    }

    // Auto-stop movement after short press
    if (move_stop_timer > 0.0)
    {
      move_stop_timer -= dt;
      if (move_stop_timer <= 0.0)
      {
        vx = vy = vyaw = 0.0;
        move_stop_timer = 0.0;
      }
    }

    switch (current_mode)
    {
      // Basic modes
      case MODE_STAND:
        sport_client.BalanceStand();
        break;

      case MODE_STAND_DOWN:
        sport_client.StandDown();
        break;

      case MODE_BALANCE_STAND:
        sport_client.BalanceStand();
        break;

      case MODE_WALK:
        sport_client.Move(vx, vy, vyaw);
        break;

      case MODE_SIT:
        sport_client.Sit();
        break;

      case MODE_DAMPING:
        sport_client.Damp();
        break;

      case MODE_RECOVERY:
        sport_client.RecoveryStand();
        break;

      // Special actions
      case MODE_HELLO:
        sport_client.Hello();
        break;

      case MODE_STRETCH:
        sport_client.Stretch();
        break;

      case MODE_CONTENT:
        sport_client.Content();
        break;

      case MODE_HEART:
        sport_client.Heart();
        break;

      case MODE_SCRAPE:
        sport_client.Scrape();
        break;

      case MODE_POSE:
        sport_client.Pose(pose_enabled);
        break;

      // Dance
      case MODE_DANCE1:
        sport_client.Dance1();
        break;

      case MODE_DANCE2:
        sport_client.Dance2();
        break;

      // Jump
      case MODE_FRONT_JUMP:
        sport_client.FrontJump();
        break;

      case MODE_FRONT_POUNCE:
        sport_client.FrontPounce();
        break;

      // Flip (dangerous)
      case MODE_FRONT_FLIP:
        sport_client.FrontFlip();
        break;

      case MODE_BACK_FLIP:
        sport_client.BackFlip();
        break;

      case MODE_LEFT_FLIP:
        sport_client.LeftFlip();
        break;

      // Gait modes
      case MODE_FREE_WALK:
        sport_client.FreeWalk();
        if (vx != 0.0 || vy != 0.0 || vyaw != 0.0)
          sport_client.Move(vx, vy, vyaw);
        break;

      case MODE_CLASSIC_WALK:
        sport_client.ClassicWalk(true);
        if (vx != 0.0 || vy != 0.0 || vyaw != 0.0)
          sport_client.Move(vx, vy, vyaw);
        break;

      case MODE_TROT_RUN:
        sport_client.TrotRun();
        if (vx != 0.0 || vy != 0.0 || vyaw != 0.0)
          sport_client.Move(vx, vy, vyaw);
        break;

      case MODE_STATIC_WALK:
        sport_client.StaticWalk();
        if (vx != 0.0 || vy != 0.0 || vyaw != 0.0)
          sport_client.Move(vx, vy, vyaw);
        break;

      case MODE_ECONOMIC_GAIT:
        sport_client.EconomicGait();
        if (vx != 0.0 || vy != 0.0 || vyaw != 0.0)
          sport_client.Move(vx, vy, vyaw);
        break;

      case MODE_FREE_BOUND:
        sport_client.FreeBound(true);
        if (vx != 0.0 || vy != 0.0 || vyaw != 0.0)
          sport_client.Move(vx, vy, vyaw);
        break;

      case MODE_FREE_JUMP:
        sport_client.FreeJump(true);
        if (vx != 0.0 || vy != 0.0 || vyaw != 0.0)
          sport_client.Move(vx, vy, vyaw);
        break;

      case MODE_FREE_AVOID:
        sport_client.FreeAvoid(true);
        if (vx != 0.0 || vy != 0.0 || vyaw != 0.0)
          sport_client.Move(vx, vy, vyaw);
        break;

      case MODE_HAND_STAND:
        sport_client.HandStand(handstand_enabled);
        if (handstand_enabled && (vx != 0.0 || vy != 0.0 || vyaw != 0.0))
          sport_client.Move(vx, vy, vyaw);
        break;

      case MODE_WALK_UPRIGHT:
        sport_client.WalkUpright(true);
        if (vx != 0.0 || vy != 0.0 || vyaw != 0.0)
          sport_client.Move(vx, vy, vyaw);
        break;

      case MODE_CROSS_STEP:
        sport_client.CrossStep(true);
        if (vx != 0.0 || vy != 0.0 || vyaw != 0.0)
          sport_client.Move(vx, vy, vyaw);
        break;

      case MODE_IDLE:
      default:
        sport_client.StopMove();
        break;
    }
  }

  void ProcessCommand(char cmd)
  {
    // Global commands (available in all menus)
    if (cmd == 'h') {
      PrintMenu();
      return;
    }
    if (cmd == 27) { // ESC
      running = false;
      std::cout << ">> 正在退出..." << std::endl;
      return;
    }
    
    // Menu switching
    if (cmd == 'm') {
      if (menu_mode == MENU_MAIN) {
        menu_mode = MENU_ACTIONS;
        std::cout << ">> 切换到【动作菜单】" << std::endl;
      } else {
        menu_mode = MENU_MAIN;
        std::cout << ">> 返回【主菜单】" << std::endl;
      }
      PrintMenu();
      return;
    }
    if (cmd == 'g') {
      if (menu_mode == MENU_MAIN) {
        menu_mode = MENU_GAIT;
        std::cout << ">> 切换到【步态菜单】" << std::endl;
      } else {
        menu_mode = MENU_MAIN;
        std::cout << ">> 返回【主菜单】" << std::endl;
      }
      PrintMenu();
      return;
    }

    // Handle commands based on current menu
    if (menu_mode == MENU_MAIN) {
      ProcessMainMenuCommand(cmd);
    } else if (menu_mode == MENU_ACTIONS) {
      ProcessActionsMenuCommand(cmd);
    } else if (menu_mode == MENU_GAIT) {
      ProcessGaitMenuCommand(cmd);
    }
  }

  void ProcessMainMenuCommand(char cmd)
  {
    switch(cmd)
    {
      case '1':  // Stand (unlocked)
        current_mode = MODE_STAND;
        vx = vy = vyaw = 0.0;
        std::cout << ">> 执行: 站立" << std::endl;
        break;

      case '2':  // Stand Down
        current_mode = MODE_STAND_DOWN;
        vx = vy = vyaw = 0.0;
        action_timer = 0.0;
        action_duration = 2.0;
        std::cout << ">> 执行: 趴下" << std::endl;
        break;

      case '3':  // Sit
        current_mode = MODE_SIT;
        vx = vy = vyaw = 0.0;
        action_timer = 0.0;
        action_duration = 2.0;
        std::cout << ">> 执行: 坐下" << std::endl;
        break;

      case '4':  // Balance Stand
        current_mode = MODE_BALANCE_STAND;
        vx = vy = vyaw = 0.0;
        std::cout << ">> 执行: 平衡站立" << std::endl;
        break;

      case '5':  // Recovery Stand
        current_mode = MODE_RECOVERY;
        vx = vy = vyaw = 0.0;
        action_timer = 0.0;
        action_duration = 3.0;
        std::cout << ">> 执行: 恢复站立" << std::endl;
        break;

      case '6':  // Damping
        current_mode = MODE_DAMPING;
        vx = vy = vyaw = 0.0;
        std::cout << ">> 执行: 阻尼模式" << std::endl;
        break;

      case '0':  // Stop/Idle
        current_mode = MODE_IDLE;
        vx = vy = vyaw = 0.0;
        std::cout << ">> 执行: 停止/待机" << std::endl;
        break;

      // Movement commands
      case 'w':
      case 's':
      case 'a':
      case 'd':
      case 'q':
      case 'e':
      case 'L':  // Left arrow
      case 'R':  // Right arrow
        HandleMovementKey(cmd);
        break;

      case 'x':
        vx = vy = vyaw = 0.0;
        move_stop_timer = 0.0;
        last_move_key = 0;
        std::cout << ">> 停止移动" << std::endl;
        break;

      default:
        break;
    }
  }

  void ProcessActionsMenuCommand(char cmd)
  {
    switch(cmd)
    {
      case '0':  // Stop
        current_mode = MODE_IDLE;
        vx = vy = vyaw = 0.0;
        std::cout << ">> 停止动作" << std::endl;
        break;

      case '1':  // Hello
        current_mode = MODE_HELLO;
        vx = vy = vyaw = 0.0;
        action_timer = 0.0;
        action_duration = 3.0;
        std::cout << ">> 执行: 打招呼" << std::endl;
        break;

      case '2':  // Stretch
        current_mode = MODE_STRETCH;
        vx = vy = vyaw = 0.0;
        action_timer = 0.0;
        action_duration = 3.0;
        std::cout << ">> 执行: 伸懒腰" << std::endl;
        break;

      case '3':  // Content
        current_mode = MODE_CONTENT;
        vx = vy = vyaw = 0.0;
        action_timer = 0.0;
        action_duration = 3.0;
        std::cout << ">> 执行: 开心" << std::endl;
        break;

      case '4':  // Heart
        current_mode = MODE_HEART;
        vx = vy = vyaw = 0.0;
        action_timer = 0.0;
        action_duration = 3.0;
        std::cout << ">> 执行: 比心" << std::endl;
        break;

      case '5':  // Scrape
        current_mode = MODE_SCRAPE;
        vx = vy = vyaw = 0.0;
        action_timer = 0.0;
        action_duration = 3.0;
        std::cout << ">> 执行: 拜年" << std::endl;
        break;

      case '6':  // Pose
        pose_enabled = !pose_enabled;
        current_mode = MODE_POSE;
        vx = vy = vyaw = 0.0;
        action_timer = 0.0;
        action_duration = 2.0;
        std::cout << ">> 执行: 摆姿势 " << (pose_enabled ? "(开启)" : "(关闭)") << std::endl;
        break;

      case '7':  // Dance1
        current_mode = MODE_DANCE1;
        vx = vy = vyaw = 0.0;
        action_timer = 0.0;
        action_duration = 10.0;
        std::cout << ">> 执行: 舞蹈1" << std::endl;
        break;

      case '8':  // Dance2
        current_mode = MODE_DANCE2;
        vx = vy = vyaw = 0.0;
        action_timer = 0.0;
        action_duration = 10.0;
        std::cout << ">> 执行: 舞蹈2" << std::endl;
        break;

      case '9':  // Front Jump
        current_mode = MODE_FRONT_JUMP;
        vx = vy = vyaw = 0.0;
        action_timer = 0.0;
        action_duration = 2.0;
        std::cout << ">> 执行: 前跳" << std::endl;
        break;

      case 'p':  // Front Pounce
        current_mode = MODE_FRONT_POUNCE;
        vx = vy = vyaw = 0.0;
        action_timer = 0.0;
        action_duration = 2.0;
        std::cout << ">> 执行: 扑人" << std::endl;
        break;

      case 'f':  // Front Flip
        std::cout << "⚠️  警告: 前空翻是危险动作，请确保周围安全!" << std::endl;
        current_mode = MODE_FRONT_FLIP;
        vx = vy = vyaw = 0.0;
        action_timer = 0.0;
        action_duration = 3.0;
        std::cout << ">> 执行: 前空翻" << std::endl;
        break;

      case 'b':  // Back Flip
        std::cout << "⚠️  警告: 后空翻是危险动作，请确保周围安全!" << std::endl;
        current_mode = MODE_BACK_FLIP;
        vx = vy = vyaw = 0.0;
        action_timer = 0.0;
        action_duration = 3.0;
        std::cout << ">> 执行: 后空翻" << std::endl;
        break;

      case 'l':  // Left Flip
        std::cout << "⚠️  警告: 左空翻是危险动作，请确保周围安全!" << std::endl;
        current_mode = MODE_LEFT_FLIP;
        vx = vy = vyaw = 0.0;
        action_timer = 0.0;
        action_duration = 3.0;
        std::cout << ">> 执行: 左空翻" << std::endl;
        break;

      default:
        break;
    }
  }

  void ProcessGaitMenuCommand(char cmd)
  {
    switch(cmd)
    {
      case '0':  // Stop
        current_mode = MODE_IDLE;
        vx = vy = vyaw = 0.0;
        std::cout << ">> 停止" << std::endl;
        break;

      case '1':  // Free Walk
        current_mode = MODE_FREE_WALK;
        vx = vy = vyaw = 0.0;
        std::cout << ">> 切换到: 灵动模式" << std::endl;
        break;

      case '2':  // Classic Walk
        current_mode = MODE_CLASSIC_WALK;
        vx = vy = vyaw = 0.0;
        std::cout << ">> 切换到: 经典步态" << std::endl;
        break;

      case '3':  // Trot Run
        current_mode = MODE_TROT_RUN;
        vx = vy = vyaw = 0.0;
        std::cout << ">> 切换到: 跑步模式" << std::endl;
        break;

      case '4':  // Static Walk
        current_mode = MODE_STATIC_WALK;
        vx = vy = vyaw = 0.0;
        std::cout << ">> 切换到: 行走模式" << std::endl;
        break;

      case '5':  // Economic Gait
        current_mode = MODE_ECONOMIC_GAIT;
        vx = vy = vyaw = 0.0;
        std::cout << ">> 切换到: 续航模式" << std::endl;
        break;

      case '6':  // Free Bound
        current_mode = MODE_FREE_BOUND;
        vx = vy = vyaw = 0.0;
        std::cout << ">> 切换到: 并腿跑" << std::endl;
        break;

      case '7':  // Free Jump
        current_mode = MODE_FREE_JUMP;
        vx = vy = vyaw = 0.0;
        std::cout << ">> 切换到: 跳跃跑" << std::endl;
        break;

      case '8':  // Free Avoid
        current_mode = MODE_FREE_AVOID;
        vx = vy = vyaw = 0.0;
        std::cout << ">> 切换到: 闪避模式" << std::endl;
        break;

      case '9':  // Hand Stand
        std::cout << "⚠️  警告: 倒立行走易导致电机过热!" << std::endl;
        handstand_enabled = !handstand_enabled;
        current_mode = MODE_HAND_STAND;
        vx = vy = vyaw = 0.0;
        std::cout << ">> 切换到: 倒立行走 " << (handstand_enabled ? "(开启)" : "(关闭)") << std::endl;
        break;

      case 'u':  // Walk Upright
        std::cout << "⚠️  警告: 直立行走易导致电机过热!" << std::endl;
        current_mode = MODE_WALK_UPRIGHT;
        vx = vy = vyaw = 0.0;
        std::cout << ">> 切换到: 直立行走" << std::endl;
        break;

      case 'c':  // Cross Step
        std::cout << "⚠️  警告: 交叉步易导致电机过热!" << std::endl;
        current_mode = MODE_CROSS_STEP;
        vx = vy = vyaw = 0.0;
        std::cout << ">> 切换到: 交叉步" << std::endl;
        break;

      // Movement commands (available in gait menu too)
      case 'w':
      case 's':
      case 'a':
      case 'd':
      case 'q':
      case 'e':
      case 'L':  // Left arrow
      case 'R':  // Right arrow
        HandleMovementKey(cmd);
        break;

      case 'x':
        vx = vy = vyaw = 0.0;
        move_stop_timer = 0.0;
        last_move_key = 0;
        std::cout << ">> 停止移动" << std::endl;
        break;

      default:
        break;
    }
  }

  bool IsMovableMode()
  {
    return current_mode == MODE_WALK || current_mode == MODE_FREE_WALK || 
           current_mode == MODE_CLASSIC_WALK || current_mode == MODE_TROT_RUN ||
           current_mode == MODE_STATIC_WALK || current_mode == MODE_ECONOMIC_GAIT ||
           current_mode == MODE_FREE_BOUND || current_mode == MODE_FREE_JUMP ||
           current_mode == MODE_FREE_AVOID || current_mode == MODE_HAND_STAND ||
           current_mode == MODE_WALK_UPRIGHT || current_mode == MODE_CROSS_STEP;
  }

  void HandleMovementKey(char key)
  {
    // Check if this is a key press (new key) or key repeat (same key)
    bool is_new_press = (last_move_key != key);
    
    if (is_new_press)
    {
      // New key pressed
      last_move_key = key;
      key_press_time = 0.0;
      
      // Switch to movable mode if not already
      if (!IsMovableMode())
      {
        current_mode = MODE_WALK;
      }
    }
    
    // Set movement based on key (AWDS layout)
    switch(key)
    {
      case 'w':  // Forward
        vx = 0.3; vy = 0.0; vyaw = 0.0;
        if (is_new_press) std::cout << ">> 前进 (vx=0.3)" << std::endl;
        break;
      case 'a':  // Left strafe
        vx = 0.0; vy = 0.2; vyaw = 0.0;
        if (is_new_press) std::cout << ">> 左移 (vy=0.2)" << std::endl;
        break;
      case 's':  // Backward
        vx = -0.3; vy = 0.0; vyaw = 0.0;
        if (is_new_press) std::cout << ">> 后退 (vx=-0.3)" << std::endl;
        break;
      case 'd':  // Right strafe
        vx = 0.0; vy = -0.2; vyaw = 0.0;
        if (is_new_press) std::cout << ">> 右移 (vy=-0.2)" << std::endl;
        break;
      case 'q':  // Turn left
      case 'L':  // Left arrow key
        vx = 0.0; vy = 0.0; vyaw = 0.5;
        if (is_new_press) std::cout << ">> 左转 (vyaw=0.5)" << std::endl;
        break;
      case 'e':  // Turn right
      case 'R':  // Right arrow key
        vx = 0.0; vy = 0.0; vyaw = -0.5;
        if (is_new_press) std::cout << ">> 右转 (vyaw=-0.5)" << std::endl;
        break;
    }
    
    // Reset auto-stop timer (for continuous movement while key is held)
    move_stop_timer = 0.5;  // Auto-stop after 0.5 seconds of no key press
  }

  void HighStateHandler(const void *message)
  {
    state = *(unitree_go::msg::dds_::SportModeState_ *)message;
  }

  int SetupKeyboard()
  {
    struct termios new_settings;
    tcgetattr(0, &initial_settings);
    new_settings = initial_settings;
    new_settings.c_lflag &= ~ICANON;
    new_settings.c_lflag &= ~ECHO;
    new_settings.c_cc[VMIN] = 0;
    new_settings.c_cc[VTIME] = 0;
    tcsetattr(0, TCSANOW, &new_settings);
    return 0;
  }

  void RestoreKeyboard()
  {
    tcsetattr(0, TCSANOW, &initial_settings);
  }

  char GetKey()
  {
    char c = 0;
    if (read(0, &c, 1) < 0)
    {
      return 0;
    }
    
    // Check for arrow keys (escape sequences)
    if (c == 27) // ESC
    {
      char seq[2];
      if (read(0, &seq[0], 1) == 1 && seq[0] == '[')
      {
        if (read(0, &seq[1], 1) == 1)
        {
          switch(seq[1])
          {
            case 'A': return 'U'; // Up arrow -> 'U'
            case 'B': return 'D'; // Down arrow -> 'D'
            case 'C': return 'R'; // Right arrow -> 'R'
            case 'D': return 'L'; // Left arrow -> 'L'
          }
        }
      }
      return 27; // Return ESC if not an arrow key
    }
    
    return c;
  }

  void Run()
  {
    SetupKeyboard();
    PrintMenu();

    unitree::common::ThreadPtr controlThreadPtr = 
      unitree::common::CreateRecurrentThread(dt * 1000000, 
        std::bind(&GO2MotionDemo::RobotControl, this));

    while(running)
    {
      char key = GetKey();
      if (key != 0)
      {
        ProcessCommand(key);
      }
      usleep(10000); // 10ms
    }

    RestoreKeyboard();
  }

  unitree_go::msg::dds_::SportModeState_ state;
  unitree::robot::go2::SportClient sport_client;
  unitree::robot::ChannelSubscriberPtr<unitree_go::msg::dds_::SportModeState_> suber;

  ControlMode current_mode;
  MenuMode menu_mode;
  double vx, vy, vyaw;
  double action_timer;
  double action_duration;
  bool pose_enabled;
  bool handstand_enabled;
  char last_move_key;
  double key_press_time;
  double move_stop_timer;
  float dt = 0.005;
  bool running = true;
  struct termios initial_settings;
};

int main(int argc, char **argv)
{
  if (argc < 2)
  {
    std::cout << "用法: " << argv[0] << " 网络接口" << std::endl;
    std::cout << "示例: " << argv[0] << " eth0" << std::endl;
    exit(-1);
  }

  std::cout << "正在初始化 GO2 运动控制演示程序..." << std::endl;
  unitree::robot::ChannelFactory::Instance()->Init(0, argv[1]);
  
  GO2MotionDemo demo;
  sleep(1);
  
  demo.Run();

  std::cout << "程序已退出。" << std::endl;
  return 0;
}
