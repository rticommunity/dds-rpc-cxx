#include "boost/make_shared.hpp"

namespace dds {
  namespace rpc {
    namespace details {

template <class Iface>
class Dispatcher;

class ServiceImplDispatcher
{
protected:
  ServiceImplBase * service_impl_ptr;

public:

  explicit ServiceImplDispatcher(ServiceImplBase &service_impl);

  ServiceImplBase * get_service_impl() const;

  virtual void run(const DDS::Duration_t &) = 0;
  virtual ~ServiceImplDispatcher();
};

class ServerImpl
{
public:

  std::vector<boost::shared_ptr<ServiceImplDispatcher>> dispatchers;
  DDS::DomainParticipant * participant_;
  DDS::Publisher * publisher_;
  DDS::Subscriber * subscriber_;

public:
  ServerImpl();

  ServerImpl(DDS::DomainParticipant * part,
             DDS::Publisher * pub,
             DDS::Subscriber * sub);

  template <class ServiceImpl>
  ServiceHandle register_service(
    ServiceImpl &service_impl,
    const std::string service_name);

  void unregister_service(int i);

  void run();
  void run(const DDS::Duration_t &);
};

class ServiceHandleImpl
{
public:
  ServiceHandleImpl(
    int i,
    ServerImpl * server);
  

  void unregister();

private:
  int idx_;
  ServerImpl * server_;
};

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

} // namespace details


template <class ServiceImpl>
ServiceHandle::ServiceHandle(ServiceImpl impl)
  : impl_(impl)
{ }

template <class ServiceImpl>
ServiceHandle Server::register_service(
  ServiceImpl &service_impl,
  const std::string service_name)
{
  return impl_->register_service(service_impl, service_name);
}


} // namespace rpc
} // namespace dds

namespace dds {

  namespace rpc {

namespace details {

class ServiceProxyImplBase
{
public:
  virtual void bind(const std::string & instance_name) = 0;
  virtual void unbind() = 0;
  virtual bool is_bound() const = 0;
  virtual const std::string & bound_instance() const = 0;
  virtual const ServiceParams & service_params() const = 0;

  virtual ~ServiceProxyImplBase();
};

template <class Iface>
class ServiceProxyImpl;

class ClientImpl
{
  DDS::DomainParticipant * participant_;
  DDS::Publisher * publisher_;
  DDS::Subscriber * subscriber_;

public:

  ClientImpl();

  ClientImpl(DDS::DomainParticipant * part,
             DDS::Publisher * pub,
             DDS::Subscriber * sub);

  template <class Iface>
  typename Iface::ProxyType
  resolve_service(const std::string & service_name);

};


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

} // namespace details 


template <class Iface>
typename Iface::ProxyType
Client::resolve_service(const std::string& service_name)
{
  return impl_->resolve_service<Iface>(service_name);
}

template <class Impl>
ServiceProxy::ServiceProxy(Impl impl)
: impl_(impl)
{}


  } // namespace rpc
} // namespace dds
