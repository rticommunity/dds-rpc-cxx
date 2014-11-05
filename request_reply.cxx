#ifdef USE_RTI_CONNEXT

#include "request_reply.h"

//#include "boost/make_shared.hpp"

//#include <map>

//#ifdef RTI_WIN32
//#define strcpy(dest, src) strcpy_s(dest, 255, src);
//#endif 

namespace dds { namespace rpc {

  RPCEntity::RPCEntity() 
  {}

  bool RPCEntity::operator == (const RPCEntity & entity)
  {
    return impl_.get() == entity.impl_.get();
  }

  void RPCEntity::close()
  {
    impl_->close();
  }

  bool RPCEntity::is_null() const
  {
    return (impl_.get() == 0);
  }

  void ServiceProxy::bind(const std::string & instance_name)
  {
    return static_cast<details::ServiceProxyImpl *>(impl_.get())->bind(instance_name);
  }

  void ServiceProxy::unbind()
  {
    return static_cast<details::ServiceProxyImpl *>(impl_.get())->unbind();
  }

  bool ServiceProxy::is_bound() const
  {
    return static_cast<const details::ServiceProxyImpl *>(impl_.get())->is_bound();
  }

  std::string ServiceProxy::get_bound_instance_name() const
  {
    return static_cast<const details::ServiceProxyImpl *>(impl_.get())->get_bound_instance_name();
  }

  std::vector<std::string> ServiceProxy::get_discoverd_service_instances() const
  {
    return static_cast<const details::ServiceProxyImpl *>(impl_.get())->get_discovered_service_instances();
  }

  RequesterParams::RequesterParams()
    : impl_(boost::make_shared<details::RequesterParamsImpl>())
  { }

  RequesterParams::RequesterParams(const RequesterParams & other)
    : impl_(boost::make_shared<details::RequesterParamsImpl>(*other.impl_.get()))
  {}

  RequesterParams & RequesterParams::operator = (const RequesterParams & that)
  {
    impl_ = boost::make_shared<details::RequesterParamsImpl>(*that.impl_.get());
    return *this;
  }

  RequesterParams & RequesterParams::domain_participant(DDSDomainParticipant * part)
  {
    impl_->domain_participant(part);
    return *this;
  }

  RequesterParams & RequesterParams::service_name(const std::string & service_name)
  {
    impl_->service_name(service_name);
    return *this;
  }

  DDSDomainParticipant * RequesterParams::domain_participant() const
  {
    return impl_->domain_participant();
  }

  std::string RequesterParams::service_name() const
  {
    return impl_->service_name();
  }

  ReplierParams::ReplierParams()
    : impl_(boost::make_shared<details::ReplierParamsImpl>())
  { }

  ReplierParams::ReplierParams(const ReplierParams & other)
    : impl_(boost::make_shared<details::ReplierParamsImpl>(*other.impl_.get()))
  {}

  ReplierParams & ReplierParams::operator = (const ReplierParams & that)
  {
    impl_ = boost::make_shared<details::ReplierParamsImpl>(*that.impl_.get());
    return *this;
  }
  
  ReplierParams & ReplierParams::domain_participant(DDSDomainParticipant * part)
  {
    impl_->domain_participant(part);
    return *this;
  }

  ReplierParams & ReplierParams::service_name(const std::string & service_name)
  {
    impl_->service_name(service_name);
    return *this;
  }

  DDSDomainParticipant * ReplierParams::domain_participant() const
  {
    return impl_->domain_participant();
  }

  std::string ReplierParams::service_name() const
  {
    return impl_->service_name();
  }


  namespace details {

    RequesterParamsImpl::RequesterParamsImpl()
      : participant_(0)
    { }

    void	RequesterParamsImpl::domain_participant(DDSDomainParticipant *participant)
    {
      participant_ = participant;
    }

    void	RequesterParamsImpl::service_name(const std::string & service_name)
    {
      service_name_ = service_name;
    }

    DDSDomainParticipant *	RequesterParamsImpl::domain_participant() const
    {
      return participant_;
    }

    std::string	RequesterParamsImpl::service_name() const
    {
      return service_name_;
    }

    ReplierParamsImpl::ReplierParamsImpl()
      : participant_(0)
    { }

    void	ReplierParamsImpl::domain_participant(DDSDomainParticipant *participant)
    {
      participant_ = participant;
    }

    void	ReplierParamsImpl::service_name(const std::string & service_name)
    {
      service_name_ = service_name;
    }

    DDSDomainParticipant *	ReplierParamsImpl::domain_participant() const
    {
      return participant_;
    }

    std::string ReplierParamsImpl::service_name() const
    {
      return service_name_;
    }

    connext::RequesterParams
      to_connext_requester_params(const dds::rpc::RequesterParams & params)
    {
        return connext::RequesterParams(
                    params.domain_participant())
              .service_name(params.service_name());
    }

    ServiceProxyImpl::~ServiceProxyImpl()
    { }

    RPCEntityImpl::~RPCEntityImpl()
    { }

} // namespace details
} // namespace rpc
} //namespace dds

#endif // USE_RTI_CONNEXT
