#ifndef OMG_DDS_RPC_ROBOT_BASIC_FUNCTION_H
#define OMG_DDS_RPC_ROBOT_BASIC_FUNCTION_H

#include "function_call.h"

#include "robotPlugin.h"
#include "robotSupport.h"

#include "request_reply.h"
#include "unique_data.h"

#define NO_EXCEPTION_SPEC

namespace robot {

class RobotControlProxy;

class RobotControl
{
  public:

  typedef RobotControlProxy    ProxyType;
  typedef RobotControl         InterfaceType;
  typedef RobotControl_Request RequestType;
  typedef RobotControl_Reply   ReplyType;

  virtual void command(const Command & command) = 0;
  virtual float setSpeed(float speed) NO_EXCEPTION_SPEC = 0;
  virtual float getSpeed() = 0;
  virtual void getStatus(Status & status) = 0;

  virtual ~RobotControl();
};

class RobotControlServiceImpl 
  : public virtual RobotControl,
    public virtual dds::rpc::ServiceImplBase
{
//  virtual void command(const Command & command) = 0;
// virtual float setSpeed(float speed) NO_EXCEPTION_SPEC = 0;
//  virtual float getSpeed() = 0;
//  virtual void getStatus(Status & status) = 0;
};


class RobotControlProxy 
  : public dds::rpc::ServiceProxy
{
  robot::RobotControl * robot_proxy_;

  public:

  typedef robot::RobotControl interface_type;

  RobotControlProxy(const ServiceProxy &);
  ~RobotControlProxy();

  void command(const Command & command);
  float setSpeed(float speed) NO_EXCEPTION_SPEC;
  float getSpeed();
  void getStatus(Status & status);
};

} // namespace robot

namespace dds {
  namespace rpc {
    namespace details {

template <>
class Dispatcher<robot::RobotControl> 
  : public details::ServiceImplDispatcher
{
  typedef robot::RobotControl                       InterfaceType;
  typedef robot::RobotControl::RequestType          RequestType;
  typedef robot::RobotControl::ReplyType            ReplyType;
  typedef dds::rpc::Replier<RequestType, ReplyType> Replier;

  boost::shared_ptr<Replier> replier_;
  robot::RobotControl * robotimpl_;

  void dispatch(const DDS::Duration_t &);

public:
  Dispatcher(ServiceImplBase & service_impl);

  virtual void run(const DDS::Duration_t &) override;

};

template <>
class ServiceProxyImpl<robot::RobotControl> 
  : public virtual details::ServiceProxyImplBase,
    public virtual robot::RobotControl
{
public:

  typedef dds::rpc::Requester<RequestType, ReplyType> Requester;

  ServiceProxyImpl(ClientImpl *, const ServiceParams &);

  void bind(const std::string & instance_name) override;
  void unbind() override;
  bool is_bound() const override;
  const std::string & bound_instance() const override;
  const ServiceParams & service_params() const override;

  void command(const robot::Command & command) override;
  float setSpeed(float speed) NO_EXCEPTION_SPEC override;
  float getSpeed() override;
  void getStatus(robot::Status & status) override;

private:
  ServiceParams params_;
  ClientImpl * client_;
  boost::shared_ptr<Requester> requester_;
};

    } // namespace details
  } // namespace rpc
} // namespace dds


#endif // OMG_DDS_RPC_ROBOT_BASIC_FUNCTION_H

