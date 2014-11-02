#include "request_reply.h"

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
  DDSDomainParticipant * participant_;
  DDSPublisher * publisher_;
  DDSSubscriber * subscriber_;

public:
  ServerImpl();

  ServerImpl(const ServerParams & server_params);

  void register_service(boost::shared_ptr<ServiceEndpointImpl> dispatcher);
  void run();
  void run(const dds::Duration_t &);
};
class ServiceParamsImpl
{
private:
  DDSDomainParticipant * participant_;
  DDSPublisher * publisher_;
  DDSSubscriber * subscriber_;
  DDS_DataWriterQos dwqos_; bool dwqos_def;
  DDS_DataReaderQos drqos_; bool drqos_def;

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
  void datawriter_qos(const dds_entity_traits::DataWriterQos qos);
  void datareader_qos(const dds_entity_traits::DataReaderQos qos);
  void publisher(DDSPublisher *publisher);
  void subscriber(DDSSubscriber *subscriber);
  void domain_participant(DDSDomainParticipant *part);

  std::string service_name() const;
  std::string instance_name() const;
  std::string request_topic_name() const;
  std::string reply_topic_name() const;
  dds_entity_traits::DataWriterQos datawriter_qos() const;
  dds_entity_traits::DataReaderQos datareader_qos() const;
  DDSPublisher * publisher() const;
  DDSSubscriber * subscriber() const;
  DDSDomainParticipant * domain_participant() const;
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
protected:
  virtual DDS::DataWriter * get_request_datawriter() const = 0;
  virtual DDS::DataReader * get_reply_datareader() const = 0;

public:
  template <class TReq>
  typename dds_type_traits<TReq>::DataWriter get_request_datawriter() const
  {
    return
      static_cast <typename dds_type_traits<TReq>::DataWriter>
        (get_request_datawriter()); // this is a polymorphic dispatch
  }

  template <class TRep>
  typename dds_type_traits<TRep>::DataReader get_reply_datareader() const
  {
    return
      static_cast <typename dds_type_traits<TRep>::DataReader>
        (get_reply_datareader()); // this is a polymorphic dispatch
  }

  virtual dds::rpc::ClientParams get_client_params() const = 0;

};

} // namespace details 


  } // namespace rpc
} // namespace dds

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

    template <class TReq>
    typename dds::dds_type_traits<TReq>::DataWriter 
      ClientEndpoint::get_request_datawriter() const
    {
        auto client_endpoint_impl =
          static_cast <details::ClientEndpointImpl *>(impl_.get());
        
        return client_endpoint_impl->get_request_datawriter<TReq>();
    }

    template <class TRep>
    typename dds::dds_type_traits<TRep>::DataReader 
      ClientEndpoint::get_reply_datareader() const
    {
        auto client_endpoint_impl =
          static_cast <details::ClientEndpointImpl *>(impl_.get());

        return client_endpoint_impl->get_request_datawriter<TRep>();
    }

  } // namespace rpc
} // namespace dds

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
