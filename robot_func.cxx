#include <stdio.h>
#include <stdlib.h>

#include "boost/make_shared.hpp"

#include "RobotControlSupport.h"

using namespace dds::rpc;
using namespace robot;

#ifdef RTI_WIN32
#define strcpy(dst, src) strcpy_s(dst, 255, src);
#endif

static const float MAX_SPEED = 100;

class MyRobot : public robot::RobotControl
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
    boost::shared_ptr<MyRobot> myrobot =
      boost::make_shared<MyRobot>("Rock-n-Rolling STATUS...");

    dds::rpc::Server server;

    RobotControlSupport::Service robot_service(*myrobot, server);

    while (true)
      server.run(dds::Duration_t::from_millis(500));
  }
  catch (std::exception & ex)
  {
    printf("Exception in server_func: %s\n", ex.what());
  }

}


void client_func(int domainid, const std::string & service_name)
{
  try {
    robot::RobotControlSupport::Client robot_client;
    robot_client.bind("robot1");
    NDDSUtility::sleep(dds::Duration_t::from_millis(1000));

    float speed = 1;

    while (true)
    {
      try 
      {
        robot_client.setSpeed(speed);
        speed = robot_client.getSpeed();
        printf("getSpeed = %f\n", speed);
        speed *= 2;
        NDDSUtility::sleep(dds::Duration_t::from_millis(1000));
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
