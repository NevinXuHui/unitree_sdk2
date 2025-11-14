/**********************************************************************
 GO2 Motion Demo - Interactive control for movement and action switching
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
  MODE_WALK,
  MODE_SIT,
  MODE_DAMPING,
  MODE_RECOVERY
};

class GO2MotionDemo
{
public:
  GO2MotionDemo()
  {
    current_mode = MODE_IDLE;
    vx = 0.0;
    vy = 0.0;
    vyaw = 0.0;
    
    sport_client.SetTimeout(10.0f);
    sport_client.Init();

    suber.reset(new unitree::robot::ChannelSubscriber<unitree_go::msg::dds_::SportModeState_>(TOPIC_HIGHSTATE));
    suber->InitChannel(std::bind(&GO2MotionDemo::HighStateHandler, this, std::placeholders::_1), 1);
  }

  void PrintMenu()
  {
    std::cout << "\n========================================" << std::endl;
    std::cout << "    GO2 Motion Control Demo" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "Action Commands:" << std::endl;
    std::cout << "  1 - Stand Up" << std::endl;
    std::cout << "  2 - Stand Down" << std::endl;
    std::cout << "  3 - Sit" << std::endl;
    std::cout << "  4 - Recovery Stand" << std::endl;
    std::cout << "  5 - Damping Mode" << std::endl;
    std::cout << "  0 - Stop/Idle" << std::endl;
    std::cout << "\nMovement Commands (in Walk mode):" << std::endl;
    std::cout << "  w - Move Forward" << std::endl;
    std::cout << "  s - Move Backward" << std::endl;
    std::cout << "  a - Turn Left" << std::endl;
    std::cout << "  d - Turn Right" << std::endl;
    std::cout << "  q - Move Left" << std::endl;
    std::cout << "  e - Move Right" << std::endl;
    std::cout << "  x - Stop Movement" << std::endl;
    std::cout << "\nOther Commands:" << std::endl;
    std::cout << "  h - Show this help" << std::endl;
    std::cout << "  ESC - Exit program" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "Current Mode: " << GetModeName() << std::endl;
    std::cout << "Current Speed: vx=" << vx << " vy=" << vy << " vyaw=" << vyaw << std::endl;
    std::cout << "========================================" << std::endl;
  }

  std::string GetModeName()
  {
    switch(current_mode)
    {
      case MODE_IDLE: return "IDLE";
      case MODE_STAND: return "STAND";
      case MODE_WALK: return "WALK";
      case MODE_SIT: return "SIT";
      case MODE_DAMPING: return "DAMPING";
      case MODE_RECOVERY: return "RECOVERY";
      default: return "UNKNOWN";
    }
  }

  void RobotControl()
  {
    switch (current_mode)
    {
      case MODE_STAND:
        sport_client.StandUp();
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

      case MODE_IDLE:
      default:
        sport_client.StopMove();
        break;
    }
  }

  void ProcessCommand(char cmd)
  {
    switch(cmd)
    {
      case '1':
        current_mode = MODE_STAND;
        vx = vy = vyaw = 0.0;
        std::cout << ">> Executing: Stand Up" << std::endl;
        break;

      case '2':
        sport_client.StandDown();
        current_mode = MODE_IDLE;
        vx = vy = vyaw = 0.0;
        std::cout << ">> Executing: Stand Down" << std::endl;
        break;

      case '3':
        current_mode = MODE_SIT;
        vx = vy = vyaw = 0.0;
        std::cout << ">> Executing: Sit" << std::endl;
        break;

      case '4':
        current_mode = MODE_RECOVERY;
        vx = vy = vyaw = 0.0;
        std::cout << ">> Executing: Recovery Stand" << std::endl;
        break;

      case '5':
        current_mode = MODE_DAMPING;
        vx = vy = vyaw = 0.0;
        std::cout << ">> Executing: Damping Mode" << std::endl;
        break;

      case '0':
        current_mode = MODE_IDLE;
        vx = vy = vyaw = 0.0;
        std::cout << ">> Executing: Stop/Idle" << std::endl;
        break;

      // Movement commands
      case 'w':
        if (current_mode != MODE_WALK)
        {
          current_mode = MODE_WALK;
          std::cout << ">> Switched to Walk Mode" << std::endl;
        }
        vx = 0.3;
        vy = 0.0;
        std::cout << ">> Moving Forward (vx=0.3)" << std::endl;
        break;

      case 's':
        if (current_mode != MODE_WALK)
        {
          current_mode = MODE_WALK;
          std::cout << ">> Switched to Walk Mode" << std::endl;
        }
        vx = -0.3;
        vy = 0.0;
        std::cout << ">> Moving Backward (vx=-0.3)" << std::endl;
        break;

      case 'a':
        if (current_mode != MODE_WALK)
        {
          current_mode = MODE_WALK;
          std::cout << ">> Switched to Walk Mode" << std::endl;
        }
        vyaw = 0.5;
        std::cout << ">> Turning Left (vyaw=0.5)" << std::endl;
        break;

      case 'd':
        if (current_mode != MODE_WALK)
        {
          current_mode = MODE_WALK;
          std::cout << ">> Switched to Walk Mode" << std::endl;
        }
        vyaw = -0.5;
        std::cout << ">> Turning Right (vyaw=-0.5)" << std::endl;
        break;

      case 'q':
        if (current_mode != MODE_WALK)
        {
          current_mode = MODE_WALK;
          std::cout << ">> Switched to Walk Mode" << std::endl;
        }
        vy = 0.2;
        vx = 0.0;
        std::cout << ">> Moving Left (vy=0.2)" << std::endl;
        break;

      case 'e':
        if (current_mode != MODE_WALK)
        {
          current_mode = MODE_WALK;
          std::cout << ">> Switched to Walk Mode" << std::endl;
        }
        vy = -0.2;
        vx = 0.0;
        std::cout << ">> Moving Right (vy=-0.2)" << std::endl;
        break;

      case 'x':
        vx = vy = vyaw = 0.0;
        if (current_mode == MODE_WALK)
        {
          std::cout << ">> Stopping Movement" << std::endl;
        }
        break;

      case 'h':
        PrintMenu();
        break;

      case 27: // ESC
        running = false;
        std::cout << ">> Exiting..." << std::endl;
        break;

      default:
        break;
    }
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
  double vx, vy, vyaw;
  float dt = 0.005;
  bool running = true;
  struct termios initial_settings;
};

int main(int argc, char **argv)
{
  if (argc < 2)
  {
    std::cout << "Usage: " << argv[0] << " networkInterface" << std::endl;
    std::cout << "Example: " << argv[0] << " eth0" << std::endl;
    exit(-1);
  }

  std::cout << "Initializing GO2 Motion Demo..." << std::endl;
  unitree::robot::ChannelFactory::Instance()->Init(0, argv[1]);
  
  GO2MotionDemo demo;
  sleep(1);
  
  demo.Run();

  std::cout << "Demo terminated." << std::endl;
  return 0;
}
