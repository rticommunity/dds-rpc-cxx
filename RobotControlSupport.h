#ifndef ROBOTCONTROL_SUPPORT_H
#define ROBOTCONTROL_SUPPORT_H

#include "function_call.h"

#include "robotPlugin.h"
#include "robotSupport.h"

namespace robot {

  class RobotControlSupport;

  class RobotControl
  {
  public:

    typedef RobotControl         InterfaceType;
    typedef RobotControlSupport  SupportType;
    typedef RobotControl_Request RequestType;
    typedef RobotControl_Reply   ReplyType;

    virtual void command(const Command & command) = 0;
    virtual float setSpeed(float speed) = 0;
    virtual float getSpeed() = 0;
    virtual void getStatus(Status & status) = 0;

    virtual ~RobotControl() { }
  };

  class RobotControlAsync
  {
  public:

    typedef RobotControl         InterfaceType;
    typedef RobotControlSupport  SupportType;
    typedef RobotControl_Request RequestType;
    typedef RobotControl_Reply   ReplyType;

    virtual dds::rpc::future<void> command_async(const robot::Command & command) = 0;
    virtual dds::rpc::future<float> setSpeed_async(float speed) = 0;
    virtual dds::rpc::future<float> getSpeed_async() = 0;
    virtual dds::rpc::future<robot::RobotControl_getStatus_Out> getStatus_async() = 0;

    virtual ~RobotControlAsync() { }
  };

  class RobotControlSupport
  {
  public:
    typedef robot::RobotControl         InterfaceType;
    typedef robot::RobotControl_Request RequestType;
    typedef robot::RobotControl_Reply   ReplyType;

    class Client : public virtual robot::RobotControl,
                   public virtual robot::RobotControlAsync,
                   public virtual dds::rpc::ClientEndpoint
    {
    public:

      Client();

      Client(const dds::rpc::ClientParams & client_params);

      /* Methods from RobotControl */
      void command(const Command & command);
      float setSpeed(float speed);
      float getSpeed();
      void getStatus(Status & status);

      /* Methods from RobotControlAsync */
      dds::rpc::future<void> command_async(const robot::Command & command);
      dds::rpc::future<float> setSpeed_async(float speed);
      dds::rpc::future<float> getSpeed_async();
      dds::rpc::future<robot::RobotControl_getStatus_Out> getStatus_async();

    };

    class Service : public dds::rpc::ServiceEndpoint
    {
    public:

      template <class Impl>
      Service(Impl & impl);

      template <class Impl>
      Service(Impl & impl, dds::rpc::Server & server);

      template <class Impl>
      Service(Impl & impl,
        dds::rpc::Server & server,
        const dds::rpc::ServiceParams & service_params);

    };

  };

} // namespace robot


#include "RobotControlSupport.hpp"

#endif // ROBOTCONTROL_SUPPORT_H
