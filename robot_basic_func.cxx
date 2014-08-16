#include "robot_basic_func.h"
#include "unique_data.h"

namespace robot {
  
RobotControl::~RobotControl()
{ }

RobotControlProxy::~RobotControlProxy()
{
  robot_proxy_ = 0;
}

RobotControlProxy::RobotControlProxy(const ServiceProxy & proxy)
: ServiceProxy(proxy),
  robot_proxy_(dynamic_cast<robot::RobotControl *>(proxy.get_impl()))
{
  if (!robot_proxy_)
    throw std::runtime_error("Not a robot::RobotControl proxy");
}

void RobotControlProxy::command(const Command & command)
{
  robot_proxy_->command(command);
}

float RobotControlProxy::setSpeed(float speed) NO_EXCEPTION_SPEC
{
  return robot_proxy_->setSpeed(speed);
}

float RobotControlProxy::getSpeed()
{
  return robot_proxy_->getSpeed();
}

void RobotControlProxy::getStatus(Status & status)
{
  robot_proxy_->getStatus(status);
}

} // namespace robot

namespace dds {
  namespace rpc {
    namespace details {

Dispatcher<robot::RobotControl>::Dispatcher(ServiceImplBase & service_impl)
  : ServiceImplDispatcher(service_impl),
    robotimpl_(0)
{
  robotimpl_ =
    dynamic_cast<robot::RobotControl *>(get_service_impl());

  if (!robotimpl_)
    throw std::runtime_error("Not a robot::RobotControl ServiceImpl");
}

bool is_command(const Sample<robot::RobotControl_Request> & request_sample)
{
  return request_sample.data().data._d == robot::RobotControl_command_hash;
}

bool is_getSpeed(const Sample<robot::RobotControl_Request> & request_sample)
{
  return request_sample.data().data._d == robot::RobotControl_getSpeed_hash;
}

bool is_setSpeed(const Sample<robot::RobotControl_Request> & request_sample)
{
  return request_sample.data().data._d == robot::RobotControl_setSpeed_hash;
}

bool is_getStatus(const Sample<robot::RobotControl_Request> & request_sample)
{
  return request_sample.data().data._d == robot::RobotControl_getStatus_hash;
}

unique_data<robot::RobotControl_Reply> do_command(
  const Sample<robot::RobotControl_Request> & request_sample,
  robot::RobotControl * service_impl)
{
  unique_data<robot::RobotControl_Reply> reply;

  service_impl->command(request_sample.data().data._u.command.com);
  
  reply->data._d = robot::RobotControl_command_hash;
  reply->data._u.command._d = dds::rpc::SUCCESS_RETCODE;
  reply->data._u.command._u.result.dummy = 0;
  
  return reply;
}

unique_data<robot::RobotControl_Reply> do_setSpeed(
  const Sample<robot::RobotControl_Request> & request_sample,
  robot::RobotControl * service_impl)
{
  unique_data<robot::RobotControl_Reply> reply;

  try
  {
    float speed =
      service_impl->setSpeed(request_sample.data().data._u.setSpeed.speed);
    //printf("do_setSpeed = %d\n", request_sample.data().data._u.setSpeed.speed);

    reply->data._d = robot::RobotControl_setSpeed_hash;
    reply->data._u.setSpeed._d = dds::rpc::SUCCESS_RETCODE;
    reply->data._u.setSpeed._u.result.return_ = speed;
  }
  catch (robot::TooFast & toofast)
  {
    reply->data._d = robot::RobotControl_setSpeed_hash;
    reply->data._u.setSpeed._d = robot::RobotControl_getSpeed_TooFast_Ex_hash;
    reply->data._u.setSpeed._u.toofast_ex = toofast;
  }

  return reply;
}

unique_data<robot::RobotControl_Reply> do_getSpeed(
  const Sample<robot::RobotControl_Request> & request_sample,
  robot::RobotControl * service_impl)
{
  unique_data<robot::RobotControl_Reply> reply;

  float speed = service_impl->getSpeed();

  reply->data._d = robot::RobotControl_getSpeed_hash;
  reply->data._u.getSpeed._d = dds::rpc::SUCCESS_RETCODE;
  reply->data._u.getSpeed._u.result.return_ = speed;

  return reply;
}

unique_data<robot::RobotControl_Reply> do_getStatus(
  const Sample<robot::RobotControl_Request> & request_sample,
  robot::RobotControl * service_impl)
{
  unique_data<robot::RobotControl_Reply> reply;

  reply->data._d = robot::RobotControl_getStatus_hash;
  reply->data._u.getStatus._d = dds::rpc::SUCCESS_RETCODE;
  service_impl->getStatus(reply->data._u.getStatus._u.result.status);

  return reply;
}

void Dispatcher<robot::RobotControl>::dispatch(const DDS::Duration_t & timeout)
{
  Sample<RequestType> request_sample;
  unique_data<ReplyType> reply;

  if (replier_->receive_request(request_sample, timeout))
  {
    if (is_command(request_sample))
      reply = do_command(request_sample, robotimpl_);
    else if (is_getSpeed(request_sample))
      reply = do_getSpeed(request_sample, robotimpl_);
    else if (is_setSpeed(request_sample))
      reply = do_setSpeed(request_sample, robotimpl_);
    else if (is_getStatus(request_sample))
      reply = do_getStatus(request_sample, robotimpl_);
    else
    {
      reply->sysx = dds::rpc::UNKNOWN_OPERATION;
      reply->data._d = 0; // default
    }

    replier_->send_reply_connext(*reply, request_sample);
  }
  else
    printf("timeout or invalid sampleinfo. Ignoring...\n");
}

void Dispatcher<robot::RobotControl>::run(const DDS::Duration_t & timeout)
{
  if (!replier_.get())
  {
    ServiceParams sp =
      ServiceImplDispatcher::get_service_impl()->get_service_params();

    replier_ = 
      boost::make_shared<Replier>(sp.domain_participant(),
                                  sp.service_name());
  }

  dispatch(timeout);
}

ServiceProxyImpl<robot::RobotControl>::ServiceProxyImpl(
  ClientImpl * client, 
  const ServiceParams & params)
: params_(params),
  client_(client),
  requester_(boost::make_shared<Requester>(params.domain_participant(), params.service_name()))
{ }

void ServiceProxyImpl<robot::RobotControl>::bind(const std::string & instance_name)
{
  params_.instance_name(instance_name);
  //requester_->bind(instance_name);
}

void ServiceProxyImpl<robot::RobotControl>::unbind()
{
  //requester_->unbind();
}

bool ServiceProxyImpl<robot::RobotControl>::is_bound() const
{
  //return requester_->is_bound();
  return false;
}

const std::string & ServiceProxyImpl<robot::RobotControl>::bound_instance() const
{
  //return requester_->bound_instance();
  return params_.instance_name();
}

const ServiceParams & ServiceProxyImpl<robot::RobotControl>::service_params() const
{
  return params_;
}

void ServiceProxyImpl<robot::RobotControl>::command(const robot::Command & command)
{
  unique_data<robot::RobotControl_Request> request;
  Sample<robot::RobotControl_Reply> reply_sample;

  request->data._d = robot::RobotControl_command_hash;
  request->data._u.command.com = command;

  requester_->send_request(*request);
  requester_->receive_reply(reply_sample, request->header.requestId);
  printf("reply received successfully from command %d\n",
         reply_sample.data().header.relatedRequestId.seqnum.low);
}

float ServiceProxyImpl<robot::RobotControl>::setSpeed(float speed) NO_EXCEPTION_SPEC
{
  unique_data<robot::RobotControl_Request> request;
  Sample<robot::RobotControl_Reply> reply_sample;

  request->data._d = robot::RobotControl_setSpeed_hash;
  request->data._u.setSpeed.speed = speed;

  requester_->send_request(*request);
  requester_->receive_reply(reply_sample, request->header.requestId);

  if (reply_sample.data().data._d == robot::RobotControl_setSpeed_hash)
  {
    printf("reply received successfully from setSpeed %d\n",
      reply_sample.data().header.relatedRequestId.seqnum.low);

    switch (reply_sample.data().data._u.setSpeed._d)
    {
      case dds::rpc::SUCCESS_RETCODE:
      {
        return reply_sample.data().data._u.setSpeed._u.result.return_;
        break;
      }
      case robot::RobotControl_getSpeed_TooFast_Ex_hash:
      {
        throw reply_sample.data().data._u.setSpeed._u.toofast_ex;
        break;
      }
      default:
      {
        throw std::runtime_error("Received unknown exception from setSpeed.");
        break;
      }
    }
  }
  else
  {
    throw std::runtime_error("Received unknown response for setSpeed.\n");
  }
}

float ServiceProxyImpl<robot::RobotControl>::getSpeed()
{
  unique_data<robot::RobotControl_Request> request;
  Sample<robot::RobotControl_Reply> reply_sample;

  request->data._d = robot::RobotControl_getSpeed_hash;
  request->data._u.getSpeed.dummy = 0;

  requester_->send_request(*request);
  requester_->receive_reply(reply_sample, request->header.requestId);

  if (reply_sample.data().data._d == robot::RobotControl_getSpeed_hash)
  {
    printf("reply received successfully from getSpeed %d\n",
      reply_sample.data().header.relatedRequestId.seqnum.low);

    switch (reply_sample.data().data._u.getSpeed._d)
    {
      case dds::rpc::SUCCESS_RETCODE:
      {
        return reply_sample.data().data._u.getSpeed._u.result.return_;
        break;
      }
      default:
      {
        throw std::runtime_error("Received unknown exception from getSpeed.");
        break;
      }
    }
  }
  else
  {
    throw std::runtime_error("Received unknown response for getSpeed.\n");
  }
}

void ServiceProxyImpl<robot::RobotControl>::getStatus(robot::Status & status)
{
  unique_data<robot::RobotControl_Request> request;
  Sample<robot::RobotControl_Reply> reply_sample;

  request->data._d = robot::RobotControl_getStatus_hash;
  request->data._u.getStatus.dummy = 0;

  requester_->send_request(*request);
  requester_->receive_reply(reply_sample, request->header.requestId);

  if (reply_sample.data().data._d == robot::RobotControl_getStatus_hash)
  {
    printf("reply received successfully from getStatus %d\n",
      reply_sample.data().header.relatedRequestId.seqnum.low);

    switch (reply_sample.data().data._u.getSpeed._d)
    {
    case dds::rpc::SUCCESS_RETCODE:
    {
      robot::Status_copy(&status, &reply_sample.data().data._u.getStatus._u.result.status);
      break;
    }
    default:
    {
      throw std::runtime_error("Received unknown exception from getStatus.");
      break;
    }
    }
  }
  else
  {
    throw std::runtime_error("Received unknown response for getStatus.\n");
  }
}

    } // namespace details
  } // namespace rpc
} // namespace dds