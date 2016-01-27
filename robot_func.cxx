#include <stdio.h>
#include <stdlib.h>
#include <numeric>

#include "boost/make_shared.hpp"

#include "RobotControlSupport.h"

using namespace dds::rpc;
using namespace robot;

#ifdef RTI_WIN32
#define strcpy(dst, src) strcpy_s(dst, 255, src);
#endif

static const float MAX_SPEED = 100;

template <class T>
T & remove_const(const T & t)
{
  return const_cast<T &>(t);
}

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

void test_conversions(robot::RobotControlSupport::Client & robot_client)
{
  dds::rpc::ClientEndpoint client_endpoint = robot_client;
  auto dw =
    client_endpoint.get_request_datawriter<robot::RobotControl::RequestType>();

  dds::rpc::ServiceProxy sp = client_endpoint;

  robot_client.bind("robot1");
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

#endif








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

dds::rpc::future<float> test_iterative_await(
  robot::RobotControlSupport::Client & robot_client)
{
  static const int increment = 1;
  float speed = 0;

  robot_client.setSpeed(0);

  while ((speed = await robot_client.getSpeed_async()) + increment <= MAX_SPEED)
  {
    float old_speed = await robot_client.setSpeed_async(speed + increment);
    printf("test_iterative_await: current speed = %f\n", old_speed + increment);
  }

  return speed;
}

#endif // USE_AWAIT











#ifdef USE_AWAIT

float add(float i, float j, float k)
{
  return i + j + k;
}

future<void> test_three_getspeed_await(robot::RobotControlSupport::Client & robot_client)
{
  printf("\n\ntest_three_getspeed_await\n");

  robot_client.setSpeed(1);

  float sum = add(await robot_client.getSpeed_async(),
                  await robot_client.getSpeed_async(),
                  await robot_client.getSpeed_async());

  printf("test_three_getspeed_await = %f\n", sum);
}












future<void> test_three_setspeed_await(robot::RobotControlSupport::Client & robot_client)
{
  printf("\n\ntest_three_setspeed_await\n");

  robot_client.setSpeed(1);

  float sum = add(await robot_client.setSpeed_async(2),
                  await robot_client.setSpeed_async(4),
                  await robot_client.setSpeed_async(6));

  printf("test_three_setspeed_await = %f\n", sum);
}













future<double> test_lambda_await(robot::RobotControlSupport::Client & robot_client)
{
  printf("\n\ntest_lambda_await\n");

  auto closure = [&robot_client]() -> future<double> { 
    await robot_client.setSpeed_async(1); 
    return 10.0;
  };
  
  return closure();
}









future<void> test_foreach_await(robot::RobotControlSupport::Client & robot_client)
{
  printf("\n\ntest_foreach_await\n");

  float speed[3] = { 5, 10, 15 };

  auto ret =  
    std::for_each(speed, speed + 3, 
                  [&robot_client](float v) -> future<void> {
                    await robot_client.setSpeed_async(v);
                  });

  return details::make_ready_future();
}





namespace test {

template <class Iter, class Init, class Func>
Init accumulate(Iter begin, Iter end, Init init, Func f)
{
  for (Iter i = begin; i < end; ++i)
  {
    init = f(std::move(init), *i);
  }

  return init;
}

} // namespace test

future<float> test_accumulate_await(robot::RobotControlSupport::Client & robot_client)
{
  printf("\n\ntest_accumulate_await\n");

  robot_client.setSpeed(0);

  float speed[3] = { 3, 5, 7 };

  auto ret = 
    test::accumulate(speed, speed + 3, 
                     details::make_ready_future<float>(0), 
                     [&robot_client](future<float> sum, float value) -> future<float> {
                        float addition = sum.get() + value;
                        await robot_client.setSpeed_async(addition);
                        return addition;
                     });

  printf("returning from test_accumulate_await\n");

  return ret;
}




template <class Iter, class Init, class Func>
future<Init> lift_accumulate(Iter begin, Iter end, Init init, Func f)
{
  auto chain = details::make_ready_future(init);

  for (Iter i = begin; i < end; ++i)
  {
    chain = 
      chain.then([f, value = *i](auto init) {
        return f(init.get(), value);
      });
  }

  return chain;
}

future<float> test_lift_accumulate_await(robot::RobotControlSupport::Client & robot_client)
{
  printf("\n\ntest_lift_accumulate_await\n");

  robot_client.setSpeed(0);

  float speed[3] = { 3, 5, 7 };

  auto ret = 
    lift_accumulate(speed, speed + 3, (float) 0,
                    [&robot_client](float sum, float value) -> future<float> {
                        float addition = sum + value;
                        await robot_client.setSpeed_async(addition);
                        return addition;
                    });

  printf("returning from test_lift_accumulate_await\n");

  return ret;
}



#endif // USE_AWAIT











void client_func(const std::string & service_name, const std::string & testname)
{
  try {
    robot::RobotControlSupport::Client robot_client;

    NDDSUtility::sleep(dds::Duration::from_millis(1000));

    if(testname.empty() || testname=="test_conversions")
      test_conversions(robot_client);
    if (testname.empty() || testname == "test_synchronous")
      test_synchronous(robot_client);
    if (testname.empty() || testname == "test_asynchronous")
      test_asynchronous(robot_client);
    if (testname.empty() || testname == "test_recursive")
      test_recursive(robot_client).get();

#ifdef USE_AWAIT
    if (testname.empty() || testname == "test_await")
      test_await(robot_client).get();
    if (testname.empty() || testname == "test_iterative_await")
      test_iterative_await(robot_client).get();
    if (testname.empty() || testname == "test_three_getspeed_await")
      test_three_getspeed_await(robot_client).get();
    if (testname.empty() || testname == "test_three_setspeed_await")
      test_three_setspeed_await(robot_client).get();
    if (testname.empty() || testname == "test_lambda_await")
      assert(test_lambda_await(robot_client).get() == 10.0);
    if (testname.empty() || testname == "test_foreach_await")
      test_foreach_await(robot_client).get();
    if (testname.empty() || testname == "test_accumulate_await")
      assert(test_accumulate_await(robot_client).get() == 15.0);
    if (testname.empty() || testname == "test_lift_accumulate_await")
    {
      auto future = test_lift_accumulate_await(robot_client);
      assert(future.is_ready() == false);
      assert(future.get() == 15.0);
    }
#endif 

    printf("Press ENTER to end the program.\n");
    getchar();
  }
  catch (...)
  {
    printf("Exception in main.\n");
  }
}

