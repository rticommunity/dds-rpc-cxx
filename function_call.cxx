#include <string>
#include <vector>
#include <stdexcept>

#include "function_call.h"
#include "boost/make_shared.hpp"

namespace dds {
namespace rpc {

namespace details {

ServerImpl::ServerImpl()
  : participant_(0),
    publisher_(0),
    subscriber_(0)
{
  participant_ =
    TheParticipantFactory->create_participant(
    0,
    DDS::PARTICIPANT_QOS_DEFAULT,
    NULL /* listener */,
    DDS::STATUS_MASK_NONE);

  if (!participant_)
    throw std::runtime_error("Unable to create participant");
}

ServerImpl::ServerImpl(DDS::DomainParticipant * part,
                        DDS::Publisher * pub,
                        DDS::Subscriber * sub)
    : participant_(part),
      publisher_(pub),
      subscriber_(sub)
{}
   
void ServerImpl::unregister_service(int i)
{
  printf("unregister requested for %s. Ignoring...\n", 
          dispatchers[i]->get_service_impl()->get_service_params().service_name().c_str());
}

void ServerImpl::run()
{
  if (!dispatchers.empty())
    dispatchers[0]->run(DDS::Duration_t::from_millis(500));
}

void ServerImpl::run(const DDS::Duration_t & timeout)
{
  if (!dispatchers.empty())
    dispatchers[0]->run(timeout);
}

ServiceHandleImpl::ServiceHandleImpl(
  int i,
  ServerImpl * server)
  : idx_(i),
    server_(server)
{}

void ServiceHandleImpl::unregister() {
  server_->unregister_service(idx_);
}


ServiceImplDispatcher::ServiceImplDispatcher(ServiceImplBase &service_impl)
  : service_impl_ptr(&service_impl)
{}

ServiceImplBase * ServiceImplDispatcher::get_service_impl() const
{
  return service_impl_ptr; 
}

ServiceImplDispatcher::~ServiceImplDispatcher()
{}


} // namespace details

Exception::Exception() 
{}

Exception::Exception(const char *msg)
: std::exception(),
  message_(msg)
{}

Exception::~Exception() throw()
{}

void ServiceHandle::unregister()
{
  impl_->unregister();
}

Server::Server()
: impl_(boost::make_shared<details::ServerImpl>())
{}

Server::Server(DDS::DomainParticipant * part,
               DDS::Publisher * pub,
               DDS::Subscriber * sub)
  : impl_(boost::make_shared<details::ServerImpl>(part, pub, sub))
{}

void Server::run()
{
  impl_->run();
}

void Server::run(const DDS::Duration_t & timeout)
{
  impl_->run(timeout);
}

const ServiceParams & ServiceImplBase::get_service_params() const
{
  return params;
}

void ServiceImplBase::set_service_params(const ServiceParams & p)
{
  params = p;
}

ServiceParams::ServiceParams()
: participant_(0),
  publisher_(0),
  subscriber_(0),
  dwqos_def(false),
  drqos_def(false),
  server_(0)
{}

ServiceParams & ServiceParams::server(Server &s)
{
  server_ = &s;
  return *this;
}

ServiceParams & ServiceParams::service_name(const std::string &service_name)
{
  service_name_ = service_name;
  return *this;
}

ServiceParams & ServiceParams::instance_name(const std::string &instance_name)
{
  instance_name_ = instance_name;
  return *this;
}

ServiceParams & ServiceParams::request_topic_name(const std::string &req_topic)
{
  request_topic_name_ = req_topic;
  return *this;
}

ServiceParams & ServiceParams::reply_topic_name(const std::string &rep_topic)
{
  reply_topic_name_ = rep_topic;
  return *this;
}

ServiceParams & ServiceParams::datawriter_qos(const DDS::DataWriterQos &qos)
{
  dwqos_ = qos;
  dwqos_def = true;
  return *this;
}

ServiceParams & ServiceParams::datareader_qos(const DDS::DataReaderQos &qos)
{
  drqos_ = qos;
  drqos_def = true;
  return *this;
}

ServiceParams & ServiceParams::publisher(DDS::Publisher *publisher)
{
  publisher_ = publisher;
  return *this;
}

ServiceParams & ServiceParams::subscriber(DDS::Subscriber *subscriber)
{
  subscriber_ = subscriber;
  return *this;
}

ServiceParams & ServiceParams::domain_participant(DDS::DomainParticipant *part)
{
  participant_ = part;
  return *this;
}

const std::string & ServiceParams::service_name() const 
{
  return service_name_;
}

const std::string & ServiceParams::instance_name() const
{
  return instance_name_;
}

const std::string & ServiceParams::request_topic_name() const
{
  return request_topic_name_;
}

const std::string & ServiceParams::reply_topic_name() const
{
  return reply_topic_name_;
}

const DDS::DataWriterQos * ServiceParams::datawriter_qos() const
{
  if (dwqos_def)
    return &dwqos_;
  else
    return 0;
}

const DDS::DataReaderQos * ServiceParams::datareader_qos() const
{
  if (drqos_def)
    return &drqos_;
  else
    return 0;
}

DDS::Publisher * ServiceParams::publisher() const
{
  return publisher_;
}

DDS::Subscriber * ServiceParams::subscriber() const
{
  return subscriber_;
}

DDS::DomainParticipant * ServiceParams::domain_participant() const
{
  return participant_;
}

Server * ServiceParams::server() const
{
  return server_;
}



} // namespace rpc
} // namespace dds

namespace dds {

namespace rpc {

  namespace details {

    ClientImpl::ClientImpl()
    {
      participant_ =
        TheParticipantFactory->create_participant(
        0,
        DDS::PARTICIPANT_QOS_DEFAULT,
        NULL /* listener */,
        DDS::STATUS_MASK_NONE);

      if (!participant_)
        throw std::runtime_error("Unable to create participant");
    }

    ClientImpl::ClientImpl(DDS::DomainParticipant * part,
                           DDS::Publisher * pub,
                           DDS::Subscriber * sub)
                           : participant_(part),
                             publisher_(pub),
                             subscriber_(sub)
    { 
      if (!participant_)
        throw std::runtime_error("Null participant");
    }

    ServiceProxyImplBase::~ServiceProxyImplBase()
    { }

} // namespace details

Client::Client()
  : impl_(boost::make_shared<details::ClientImpl>())
{}

Client::Client(DDS::DomainParticipant * part,
               DDS::Publisher * pub,
               DDS::Subscriber * sub )
  : impl_(boost::make_shared<details::ClientImpl>(part, pub, sub))
{}

void ServiceProxy::bind(const std::string & instance_name)
{
  impl_->bind(instance_name);
}

void ServiceProxy::unbind()
{
  impl_->unbind();
}

bool ServiceProxy::is_bound() const
{
  return impl_->is_bound();
}

const std::string & ServiceProxy::bound_instance() const
{
  return impl_->bound_instance();
}

const ServiceParams & ServiceProxy::service_params() const
{
  return impl_->service_params();
}

details::ServiceProxyImplBase * ServiceProxy::get_impl() const
{
  return impl_.get();
}

} // namespace rpc

}  // namespace dds

