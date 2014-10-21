#include "request_reply.h"

namespace dds {
  namespace rpc {

    template <class Impl>
    ServiceEndpoint::ServiceEndpoint(Impl impl)
      : impl_(impl)
    { }

    template <class Impl>
    ClientEndpoint::ClientEndpoint(Impl impl)
      : ServiceProxy(impl)
    { }


  } // namespace rpc
} // namespace dds

namespace dds {
  namespace rpc {
    namespace details {

template <class Iface>
class Dispatcher;

class ServiceEndpointImpl
{
public:

  virtual void run(const dds::Duration_t &) = 0;
  virtual ~ServiceEndpointImpl();
};

class ServerImpl
{
public:

  std::vector<boost::shared_ptr<ServiceEndpointImpl>> dispatchers;
  dds::DomainParticipant * participant_;
  dds::Publisher * publisher_;
  dds::Subscriber * subscriber_;

public:
  ServerImpl();

  ServerImpl(const ServerParams & server_params);

  void register_service(boost::shared_ptr<ServiceEndpointImpl> dispatcher);
  void run();
  void run(const dds::Duration_t &);
};
/*

template <class ServiceImpl>
ServiceHandle ServerImpl::register_service(
  ServiceImpl &service_impl,
  const std::string service_name)
{
  auto shptr =
    boost::make_shared <
      details::Dispatcher<typename ServiceImpl::InterfaceType >>
        (service_impl);

  shptr->get_service_impl()->set_service_params(
    ServiceParams()
      .domain_participant(this->participant_)
      .service_name(service_name));
  
  dispatchers.push_back(shptr);

  return ServiceHandle(boost::make_shared<ServiceHandleImpl>(0, this));
}
*/
class ServiceParamsImpl
{
private:
  dds::DomainParticipant * participant_;
  dds::Publisher * publisher_;
  dds::Subscriber * subscriber_;
  dds::DataWriterQos dwqos_; bool dwqos_def;
  dds::DataReaderQos drqos_; bool drqos_def;

  std::string service_name_;
  std::string instance_name_;
  std::string request_topic_name_;
  std::string reply_topic_name_;

public:
  ServiceParamsImpl();

  void service_name(const std::string &service_name);
  void instance_name(const std::string &instance_name);
  void request_topic_name(const std::string &req_topic);
  void reply_topic_name(const std::string &rep_topic);
  void datawriter_qos(const dds::DataWriterQos &qos);
  void datareader_qos(const dds::DataReaderQos &qos);
  void publisher(dds::Publisher *publisher);
  void subscriber(dds::Subscriber *subscriber);
  void domain_participant(dds::DomainParticipant *part);

  std::string service_name() const;
  std::string instance_name() const;
  std::string request_topic_name() const;
  std::string reply_topic_name() const;
  const dds::DataWriterQos * datawriter_qos() const;
  const dds::DataReaderQos * datareader_qos() const;
  dds::Publisher * publisher() const;
  dds::Subscriber * subscriber() const;
  dds::DomainParticipant * domain_participant() const;
};

class ClientParamsImpl : public ServiceParamsImpl
{
public:
  ClientParamsImpl();
};

class ServerParamsImpl
{
  dds::rpc::ServiceParams service_params_;

public:
  ServerParamsImpl();

  void default_service_params(const ServiceParams & service_params);

  ServiceParams default_service_params() const;
};


} // namespace details

} // namespace rpc
} // namespace dds

namespace dds {

  namespace rpc {

    namespace details {
      
class ClientEndpointImpl : public ServiceProxyImpl
{
public:
  template <class TReq>
  typename dds_type_traits<TReq>::DataWriter get_request_datawriter() const;

  template <class TRep>
  typename dds_type_traits<TRep>::DataReader get_reply_datareader() const;

  virtual dds::rpc::ClientParams get_client_params() const = 0;

};

/*
template <class Iface>
typename Iface::ProxyType
ClientImpl::resolve_service(const std::string & service_name)
{
  return ServiceProxy(
    boost::make_shared <details::ServiceProxyImpl<Iface>>(
      this,
      ServiceParams().domain_participant(participant_)
                     .service_name(service_name)));
}
*/
} // namespace details 


  } // namespace rpc
} // namespace dds
