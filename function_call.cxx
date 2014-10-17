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
   
void ServerImpl::register_service(boost::shared_ptr<ServiceEndpointImpl> dispatcher)
{
  dispatchers.push_back(dispatcher);
  printf("register service...\n");
}
/*
void ServerImpl::unregister_service(int i)
{
  printf("unregister requested for %s. Ignoring...\n",
    dispatchers[i]->get_service_impl()->get_service_params().service_name().c_str());
}
*/
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

boost::shared_ptr<details::ServerImpl> Server::get_impl() const
{
  return impl_;
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
: impl_(boost::make_shared<details::ServiceParamsImpl>())
{}

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

ServiceParams & ServiceParams::datawriter_qos(const DDS::DataWriterQos &qos)
{
  impl_->datawriter_qos(qos);
  return *this;
}

ServiceParams & ServiceParams::datareader_qos(const DDS::DataReaderQos &qos)
{
  impl_->datareader_qos(qos);
  return *this;
}

ServiceParams & ServiceParams::publisher(DDS::Publisher *publisher)
{
  impl_->publisher(publisher);
  return *this;
}

ServiceParams & ServiceParams::subscriber(DDS::Subscriber *subscriber)
{
  impl_->subscriber(subscriber);
  return *this;
}

ServiceParams & ServiceParams::domain_participant(DDS::DomainParticipant *part)
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

const DDS::DataWriterQos * ServiceParams::datawriter_qos() const
{
  return impl_->datawriter_qos();
}

const DDS::DataReaderQos * ServiceParams::datareader_qos() const
{
  return impl_->datareader_qos();
}

DDS::Publisher * ServiceParams::publisher() const
{
  return impl_->publisher();
}

DDS::Subscriber * ServiceParams::subscriber() const
{
  return impl_->subscriber();
}

DDS::DomainParticipant * ServiceParams::domain_participant() const
{
  return impl_->domain_participant();
}

ClientParams::ClientParams()
: impl_(boost::make_shared<details::ClientParamsImpl>())
{ }


} // namespace rpc
} // namespace dds

namespace dds {

namespace rpc {

  namespace details {

    ClientParamsImpl::ClientParamsImpl()
    {}

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

    void ServiceParamsImpl::datawriter_qos(const DDS::DataWriterQos &qos)
    {
      dwqos_ = qos;
      dwqos_def = true;
    }

    void ServiceParamsImpl::datareader_qos(const DDS::DataReaderQos &qos)
    {
      drqos_ = qos;
      drqos_def = true;
    }

    void ServiceParamsImpl::publisher(DDS::Publisher *publisher)
    {
      publisher_ = publisher;
    }

    void ServiceParamsImpl::subscriber(DDS::Subscriber *subscriber)
    {
      subscriber_ = subscriber;
    }

    void ServiceParamsImpl::domain_participant(DDS::DomainParticipant *part)
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

    const DDS::DataWriterQos * ServiceParamsImpl::datawriter_qos() const
    {
      if (dwqos_def)
        return &dwqos_;
      else
        return 0;
    }

    const DDS::DataReaderQos * ServiceParamsImpl::datareader_qos() const
    {
      if (drqos_def)
        return &drqos_;
      else
        return 0;
    }

    DDS::Publisher * ServiceParamsImpl::publisher() const
    {
      return publisher_;
    }

    DDS::Subscriber * ServiceParamsImpl::subscriber() const
    {
      return subscriber_;
    }

    DDS::DomainParticipant * ServiceParamsImpl::domain_participant() const
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
    */
} // namespace details

} // namespace rpc

}  // namespace dds

