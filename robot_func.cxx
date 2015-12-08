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

void server_func(const std::string & service_name)
{
  try {
    boost::shared_ptr<MyRobot> myrobot1 =
      boost::make_shared<MyRobot>("Rock-n-Rolling STATUS...");

    boost::shared_ptr<MyRobot> myrobot2 =
      boost::make_shared<MyRobot>("Cool STATUS...");

    dds::rpc::Server server;

    RobotControlSupport::Service 
      robot_service(*myrobot1, 
                    server, 
                    dds::rpc::ServiceParams().service_name(service_name).instance_name("Rock"));
    
    dds::rpc::ServiceEndpoint se = robot_service;

    while (true)
      server.run(dds::Duration::from_seconds(20));
  }
  catch (std::exception & ex)
  {
    printf("Exception in server_func: %s\n", ex.what());
  }

}

template <class T>
T & remove_const(const T & t)
{
  return const_cast<T &>(t);
}

dds::rpc::future<float> speedup_until_maxspeed(
  robot::RobotControlSupport::Client & robot_client)
{
  static const int increment = 10;

  return
    robot_client
      .getSpeed_async()
      .then([robot_client](future<float> && speed_fut) 
      {
        float speed = speed_fut.get();
        if (speed + increment <= MAX_SPEED)
        {
          printf("speedup_until_maxspeed: new speed = %f\n", speed + increment);
          return remove_const(robot_client).setSpeed_async(speed + increment);
        }
        else
          return dds::rpc::details::make_ready_future(speed);
      })
      .then([robot_client](future<float> && speed_fut) {
        float speed = speed_fut.get();
        if (speed + increment <= MAX_SPEED)
          return speedup_until_maxspeed(remove_const(robot_client));
        else
          return dds::rpc::details::make_ready_future(speed);
      });
}

dds::rpc::future<float> test_recursive(
  robot::RobotControlSupport::Client & robot_client)
{
  robot_client.setSpeed(0);

  return speedup_until_maxspeed(robot_client);
}

#ifdef USE_AWAIT

dds::rpc::future<void> test_iterative_await(
  robot::RobotControlSupport::Client & robot_client)
{
  static const int increment = 1;
  float speed = 0;

  robot_client.setSpeed(0);

  while ((speed = await robot_client.getSpeed_async()) + increment <= MAX_SPEED)
  {
    printf("test_iterative_await: thread id = %lld\n", RTIOsapiThread_getCurrentThreadID());
    await robot_client.setSpeed_async(speed + increment);
    printf("test_iterative_await: current speed = %f, thread id = %lld\n", 
           speed + increment, RTIOsapiThread_getCurrentThreadID());
  }
}

#endif // USE_AWAIT

void test_asynchronous(robot::RobotControlSupport::Client & robot_client)
{
  try {

    for (int i = 0; i < 10; i++)
    {
      robot_client
        .getSpeed_async()
        .then([robot_client](future<float> && speed_fut) {
              printf("Callback1 thread id = %lld\n", RTIOsapiThread_getCurrentThreadID());
              float speed = speed_fut.get();
              printf("test_asynchronous: getSpeed = %f\n", speed);
              speed *= 2;
              return remove_const(robot_client).setSpeed_async(speed);
          })
        .then([](future<float> && speed_fut) {
            try {
              printf("Callback2 thread id = %lld\n", RTIOsapiThread_getCurrentThreadID());
              float speed = speed_fut.get();
            }
            catch (TooFast &)
            {
              printf("test_asynchronous: Going too fast!\n");
            }
          });

      NDDSUtility::sleep(dds::Duration::from_millis(100));
    }
  }
  catch (std::exception & ex)
  {
    printf("Exception in client_func: %s\n", ex.what());
  }
  catch (...)
  {
    printf("test_asynchronous: Unknown exception\n");
  }
}

void test_synchronous(robot::RobotControlSupport::Client & robot_client)
{
  try {
    float speed = 1;

    for (int i = 0; i < 10; i++)
    {
      try
      {
        robot_client.setSpeed(speed);
        speed = robot_client.getSpeed();
        printf("getSpeed = %f\n", speed);
        speed *= 2;
        //NDDSUtility::sleep(dds::Duration::from_millis(1000));
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

void test_conversions(robot::RobotControlSupport::Client & robot_client)
{
  dds::rpc::ClientEndpoint client_endpoint = robot_client;
  auto dw =
    client_endpoint.get_request_datawriter<robot::RobotControl::RequestType>();

  dds::rpc::ServiceProxy sp = client_endpoint;

  robot_client.bind("robot1");
}

#ifdef USE_AWAIT

future<void> test_await(robot::RobotControlSupport::Client & robot_client)
{
  robot_client.setSpeed_async(1);

  for (int i = 0; i < 10; i++)
  {
    float speed = await robot_client.getSpeed_async();
    float oldspeed = await robot_client.setSpeed_async(speed + 2);
    assert(speed == oldspeed);
    printf("test_await: old speed = %f\n", oldspeed);
  }
}

#endif // USE_AWAIT

void client_func(const std::string & service_name)
{
  try {
    robot::RobotControlSupport::Client robot_client;

    NDDSUtility::sleep(dds::Duration::from_millis(1000));

    test_conversions(robot_client);
    test_synchronous(robot_client);
    test_asynchronous(robot_client);
    test_recursive(robot_client).get();

#ifdef USE_AWAIT
    //test_await(robot_client).get();
    test_iterative_await(robot_client).get();
#endif 

    printf("Press ENTER to end the program.\n");
    getchar();
  }
  catch (...)
  {
    printf("Exception in main.\n");
  }
}

