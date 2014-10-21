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

ServerImpl::ServerImpl(const ServerParams & sp)
    : participant_(sp.default_service_params().domain_participant()),
      publisher_(sp.default_service_params().publisher()),
      subscriber_(sp.default_service_params().subscriber())
{}
   
void ServerImpl::register_service(boost::shared_ptr<ServiceEndpointImpl> dispatcher)
{
  dispatchers.push_back(dispatcher);
  printf("register service...\n");
}

void ServerImpl::run()
{
  if (!dispatchers.empty())
    dispatchers[0]->run(dds::Duration_t::from_millis(500));
}

void ServerImpl::run(const dds::Duration_t & timeout)
{
  if (!dispatchers.empty())
    dispatchers[0]->run(timeout);
}

ServiceEndpointImpl::~ServiceEndpointImpl()
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

Server::Server()
: impl_(boost::make_shared<details::ServerImpl>())
{}

Server::Server(const ServerParams & server_params)
  : impl_(boost::make_shared<details::ServerImpl>(server_params))
{}

void Server::run()
{
  impl_->run();
}

void Server::run(const dds::Duration_t & timeout)
{
  impl_->run(timeout);
}

boost::shared_ptr<details::ServerImpl> Server::get_impl() const
{
  return impl_;
}

ServerParams::ServerParams()
: impl_(boost::make_shared<details::ServerParamsImpl>())
{}

ServerParams::ServerParams(const ServerParams & other)
: impl_(boost::make_shared<details::ServerParamsImpl>(*other.impl_.get()))
{}

ServerParams & ServerParams::operator = (const ServerParams & that)
{
  impl_ = boost::make_shared<details::ServerParamsImpl>(*that.impl_.get());
  return *this;
}

ServerParams & ServerParams::default_service_params(const ServiceParams & service_params)
{
  impl_->default_service_params(service_params);
  return *this;
}

ServiceParams ServerParams::default_service_params() const
{
  return impl_->default_service_params();
}

ServiceParams::ServiceParams()
: impl_(boost::make_shared<details::ServiceParamsImpl>())
{}


ServiceParams::ServiceParams(const ServiceParams & other)
: impl_(boost::make_shared<details::ServiceParamsImpl>(*other.impl_.get()))
{}

ServiceParams & ServiceParams::operator = (const ServiceParams & that)
{
  impl_ = boost::make_shared<details::ServiceParamsImpl>(*that.impl_.get());
  return *this;
}

ServiceParams & ServiceParams::service_name(const std::string &service_name)
{
  impl_->service_name(service_name);
  return *this;
}

ServiceParams & ServiceParams::instance_name(const std::string &instance_name)
{
  impl_->instance_name(instance_name);
  return *this;
}

ServiceParams & ServiceParams::datawriter_qos(const dds::DataWriterQos &qos)
{
  impl_->datawriter_qos(qos);
  return *this;
}

ServiceParams & ServiceParams::datareader_qos(const dds::DataReaderQos &qos)
{
  impl_->datareader_qos(qos);
  return *this;
}

ServiceParams & ServiceParams::publisher(dds::Publisher *publisher)
{
  impl_->publisher(publisher);
  return *this;
}

ServiceParams & ServiceParams::subscriber(dds::Subscriber *subscriber)
{
  impl_->subscriber(subscriber);
  return *this;
}

ServiceParams & ServiceParams::domain_participant(dds::DomainParticipant *part)
{
  impl_->domain_participant(part);
  return *this;
}

std::string ServiceParams::service_name() const 
{
  return impl_->service_name();
}

std::string ServiceParams::instance_name() const
{
  return impl_->instance_name();
}

std::string ServiceParams::request_topic_name() const
{
  return impl_->request_topic_name();
}

std::string ServiceParams::reply_topic_name() const
{
  return impl_->reply_topic_name();
}

const dds::DataWriterQos * ServiceParams::datawriter_qos() const
{
  return impl_->datawriter_qos();
}

const dds::DataReaderQos * ServiceParams::datareader_qos() const
{
  return impl_->datareader_qos();
}

dds::Publisher * ServiceParams::publisher() const
{
  return impl_->publisher();
}

dds::Subscriber * ServiceParams::subscriber() const
{
  return impl_->subscriber();
}

dds::DomainParticipant * ServiceParams::domain_participant() const
{
  return impl_->domain_participant();
}

ClientParams::ClientParams()
: impl_(boost::make_shared<details::ClientParamsImpl>())
{ }


ClientParams::ClientParams(const ClientParams & other)
: impl_(boost::make_shared<details::ClientParamsImpl>(*other.impl_.get()))
{}

ClientParams & ClientParams::operator = (const ClientParams & that)
{
  impl_ = boost::make_shared<details::ClientParamsImpl>(*that.impl_.get());
  return *this;
}


} // namespace rpc
} // namespace dds

namespace dds {

namespace rpc {

  namespace details {

    ClientParamsImpl::ClientParamsImpl()
    {}

    ServerParamsImpl::ServerParamsImpl()
    {}

    void ServerParamsImpl::default_service_params(const ServiceParams & service_params)
    {
      service_params_ = service_params;
    }

    ServiceParams ServerParamsImpl::default_service_params() const
    {
      return service_params_;
    }

    ServiceParamsImpl::ServiceParamsImpl()
      : participant_(0),
        publisher_(0),
        subscriber_(0),
        dwqos_def(false),
        drqos_def(false)
    {}

    void ServiceParamsImpl::service_name(const std::string &service_name)
    {
      service_name_ = service_name;
    }

    void ServiceParamsImpl::instance_name(const std::string &instance_name)
    {
      instance_name_ = instance_name;
    }

    void ServiceParamsImpl::request_topic_name(const std::string &req_topic)
    {
      request_topic_name_ = req_topic;
    }

    void ServiceParamsImpl::reply_topic_name(const std::string &rep_topic)
    {
      reply_topic_name_ = rep_topic;
    }

    void ServiceParamsImpl::datawriter_qos(const dds::DataWriterQos &qos)
    {
      dwqos_ = qos;
      dwqos_def = true;
    }

    void ServiceParamsImpl::datareader_qos(const dds::DataReaderQos &qos)
    {
      drqos_ = qos;
      drqos_def = true;
    }

    void ServiceParamsImpl::publisher(dds::Publisher *publisher)
    {
      publisher_ = publisher;
    }

    void ServiceParamsImpl::subscriber(dds::Subscriber *subscriber)
    {
      subscriber_ = subscriber;
    }

    void ServiceParamsImpl::domain_participant(dds::DomainParticipant *part)
    {
      participant_ = part;
    }

    std::string ServiceParamsImpl::service_name() const
    {
      return service_name_;
    }

    std::string ServiceParamsImpl::instance_name() const
    {
      return instance_name_;
    }

    std::string ServiceParamsImpl::request_topic_name() const
    {
      return request_topic_name_;
    }

    std::string ServiceParamsImpl::reply_topic_name() const
    {
      return reply_topic_name_;
    }

    const dds::DataWriterQos * ServiceParamsImpl::datawriter_qos() const
    {
      if (dwqos_def)
        return &dwqos_;
      else
        return 0;
    }

    const dds::DataReaderQos * ServiceParamsImpl::datareader_qos() const
    {
      if (drqos_def)
        return &drqos_;
      else
        return 0;
    }

    dds::Publisher * ServiceParamsImpl::publisher() const
    {
      return publisher_;
    }

    dds::Subscriber * ServiceParamsImpl::subscriber() const
    {
      return subscriber_;
    }

    dds::DomainParticipant * ServiceParamsImpl::domain_participant() const
    {
      return participant_;
    }


    /*
    ClientImpl::ClientImpl()
    {
      participant_ =
        TheParticipantFactory->create_participant(
        0,
        DDS::PARTICIPANT_QOS_DEFAULT,
        NULL ,
        DDS::STATUS_MASK_NONE);

      if (!participant_)
        throw std::runtime_error("Unable to create participant");
    }

    ClientImpl::ClientImpl(dds::DomainParticipant * part,
                           dds::Publisher * pub,
                           dds::Subscriber * sub)
                           : participant_(part),
                             publisher_(pub),
                             subscriber_(sub)
    { 
      if (!participant_)
        throw std::runtime_error("Null participant");
    }
    */
} // namespace details

} // namespace rpc

}  // namespace dds

