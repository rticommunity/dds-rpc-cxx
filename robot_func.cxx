#include <stdio.h>
#include <stdlib.h>

#include "boost/make_shared.hpp"

#include "robot_basic_func.h"

using namespace dds::rpc;
using namespace robot;

#define NO_EXCEPTION_SPEC

#ifdef RTI_WIN32
#define strcpy(dst, src) strcpy_s(dst, 255, src);
#endif

static const float MAX_SPEED = 100;

class MyRobot : public robot::RobotControlServiceImpl
{
  float speed_;
  Status status_;

public:

  MyRobot(const char * msg = 0)
    : speed_(10)
  {
    robot::Status_initialize(&status_);
    if (msg)
      strcpy(status_.msg, msg);
  }

  ~MyRobot()
  {
    robot::Status_finalize(&status_);
  }

  void command(const Command & com) override
  {
    switch (com)
    {
    case robot::START_COMMAND:
      printf("command callback: START_COMMAND\n");
      break;
    case robot::STOP_COMMAND:
      printf("command callback: STOP_COMMAND\n");
      break;
    default:
      printf("Unknown\n");
    }
  }

  float setSpeed(float speed) override
  {
    printf("setSpeed = %f\n", speed);
    float oldspeed = speed_;

    if (speed <= MAX_SPEED)
      speed_ = speed;
    else
      throw TooFast();

    return oldspeed;
  }

  float getSpeed() override
  {
    printf("getSpeed\n");
    return speed_;
  }

  void getStatus(Status & status) override
  {
    printf("getStatus\n");
    //FIXME: status = status_;
    if (status.msg)
      Status_copy(&status, &status_);
  }
};

void server_func(int domainid, const std::string & service_name)
{
  try {
    dds::rpc::Server s;

    boost::shared_ptr<MyRobot> myrobot =
      boost::make_shared<MyRobot>("Rock-n-Rolling STATUS...");

    ServiceHandle robot_handle =
      s.register_service(*myrobot, service_name);
    
    while (true)
      s.run(DDS::Duration_t::from_millis(500));
  }
  catch (std::exception & ex)
  {
    printf("Exception in server_func: %s\n", ex.what());
  }

}


void client_func(int domainid, const std::string & service_name)
{
  try {
    dds::rpc::Client client;
    //ServiceProxy sp = client.resolve_service<robot::RobotControl>(service_name);
    robot::RobotControlProxy robotcontrol = 
      client.resolve_service<robot::RobotControl>(service_name);
    robotcontrol.bind("robot1");
    NDDSUtility::sleep(DDS::Duration_t::from_millis(1000));
    //robot::RobotControlProxy robotcontrol(sp);

    float speed = 1;

    while (true)
    {
      try 
      {
        robotcontrol.setSpeed(speed);
        speed = robotcontrol.getSpeed();
        printf("getSpeed = %f\n", speed);
        speed *= 2;
        NDDSUtility::sleep(DDS::Duration_t::from_millis(1000));
      }
      catch (robot::TooFast &) {
        speed = 1;
      }
    }
  }
  catch (std::exception & ex)
  {
    printf("Exception in client_func: %s\n", ex.what());
  }
}
