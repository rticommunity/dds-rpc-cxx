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
   
void ServerImpl::register_service(boost::shared_ptr<RPCEntityImpl> dispatcher)
{
  dispatchers.push_back(dispatcher);
  printf("register service...\n");
}

void ServerImpl::close()
{}

void ServerImpl::run()
{
  if (!dispatchers.empty())
    static_cast<ServerImpl *>(dispatchers[0].get())->run(dds::Duration::from_millis(500));
}

void ServerImpl::run(const dds::Duration & timeout)
{
  if (!dispatchers.empty())
    static_cast<ServerImpl *>(dispatchers[0].get())->run(timeout);
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
: RPCEntity(boost::make_shared<details::ServerImpl>())
{}

Server::Server(const ServerParams & server_params)
  : RPCEntity(boost::make_shared<details::ServerImpl>(server_params))
{}

Server::VendorDependent Server::get_impl() const
{
  return boost::dynamic_pointer_cast<details::ServerImpl>(impl_);
}

void Server::run()
{
  static_cast<details::ServerImpl *>(impl_.get())->run();
}

void Server::run(const dds::Duration & timeout)
{
  static_cast<details::ServerImpl *>(impl_.get())->run(timeout);
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

ServiceParams & ServiceParams::datawriter_qos(dds_entity_traits::DataWriterQos qos)
{
  impl_->datawriter_qos(qos);
  return *this;
}

ServiceParams & ServiceParams::datareader_qos(dds_entity_traits::DataReaderQos qos)
{
  impl_->datareader_qos(qos);
  return *this;
}

ServiceParams & ServiceParams::publisher(dds_entity_traits::Publisher publisher)
{
  impl_->publisher(publisher);
  return *this;
}

ServiceParams & ServiceParams::subscriber(dds_entity_traits::Subscriber subscriber)
{
  impl_->subscriber(subscriber);
  return *this;
}

ServiceParams & ServiceParams::domain_participant(dds_entity_traits::DomainParticipant part)
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

dds_entity_traits::DataWriterQos ServiceParams::datawriter_qos() const
{
  return impl_->datawriter_qos();
}

dds_entity_traits::DataReaderQos ServiceParams::datareader_qos() const
{
  return impl_->datareader_qos();
}

dds_entity_traits::Publisher ServiceParams::publisher() const
{
  return impl_->publisher();
}

dds_entity_traits::Subscriber ServiceParams::subscriber() const
{
  return impl_->subscriber();
}

dds_entity_traits::DomainParticipant ServiceParams::domain_participant() const
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

    void ServiceParamsImpl::datawriter_qos(const dds_entity_traits::DataWriterQos qos)
    {
      dwqos_ = *qos;
      dwqos_def = true;
    }

    void ServiceParamsImpl::datareader_qos(dds_entity_traits::DataReaderQos qos)
    {
      drqos_ = *qos;
      drqos_def = true;
    }

    void ServiceParamsImpl::publisher(DDSPublisher *publisher)
    {
      publisher_ = publisher;
    }

    void ServiceParamsImpl::subscriber(DDSSubscriber *subscriber)
    {
      subscriber_ = subscriber;
    }

    void ServiceParamsImpl::domain_participant(DDSDomainParticipant *part)
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

    dds_entity_traits::DataWriterQos ServiceParamsImpl::datawriter_qos() const
    {
      if (dwqos_def)
        return &dwqos_;
      else
        return 0;
    }

    dds_entity_traits::DataReaderQos ServiceParamsImpl::datareader_qos() const
    {
      if (drqos_def)
        return &drqos_;
      else
        return 0;
    }

    DDSPublisher * ServiceParamsImpl::publisher() const
    {
      return publisher_;
    }

    DDSSubscriber * ServiceParamsImpl::subscriber() const
    {
      return subscriber_;
    }

    dds_entity_traits::DomainParticipant ServiceParamsImpl::domain_participant() const
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

