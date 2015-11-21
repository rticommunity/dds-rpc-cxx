#ifdef USE_RTI_CONNEXT

#include "connext_cpp/connext_cpp_requester.h"
#include "connext_cpp/connext_cpp_replier.h"
#include "boost/make_shared.hpp"

#include <map>

#include "common.h"

#ifdef RTI_WIN32
#define strcpy(dest, src) strcpy_s(dest, 255, src);
#endif 

namespace dds { 
namespace rpc {
namespace details {

class RPCEntityImpl
{
public:
  virtual void close() = 0;

  virtual ~RPCEntityImpl();
};

class ServiceProxyImpl : public RPCEntityImpl
{
public:

  virtual void bind(const std::string & instance_name) = 0;
  virtual void unbind() = 0;
  virtual bool is_bound() const = 0;
  virtual std::string get_bound_instance_name() const = 0;
  virtual std::vector<std::string> get_discovered_service_instances() const = 0;

  virtual void wait_for_service() = 0;
  virtual void wait_for_service(const dds::Duration & maxWait) = 0;

  virtual void wait_for_service(std::string instanceName) = 0;
  virtual void wait_for_service(const dds::Duration & maxWait,
                                std::string instanceName) = 0;

  virtual void wait_for_services(int count) = 0;
  virtual void wait_for_services(const dds::Duration & maxWait, int count) = 0;

  virtual void wait_for_services(const std::vector<std::string> & instanceNames) = 0;
  virtual void wait_for_services(const dds::Duration & maxWait,
                                 const std::vector<std::string> & instanceNames) = 0;

  virtual future<void> wait_for_service_async() = 0;
  virtual future<void> wait_for_service_async(std::string instanceName) = 0;
  virtual future<void> wait_for_services_async(int count) = 0;
  virtual future<void> wait_for_services_async(
    const std::vector<std::string> & instanceNames) = 0;

  virtual ~ServiceProxyImpl();
};

connext::RequesterParams
  to_connext_requester_params(
    const dds::rpc::RequesterParams & params);

template <class TReq, class TRep>
class RequesterImpl : public details::ServiceProxyImpl,
                      public connext::Requester<TReq, TRep>
{
  private:
    std::string service_name_;
    std::string instance_name_;
    long sn;
    bool suppress_invalid;
    std::map<DDS::SampleIdentity_t, promise<Sample<TRep>>> dict;
    std::map<dds::SampleIdentity, DDS::SampleIdentity_t> id2id_map;

    typedef connext::Requester<TReq, TRep> super;

    struct SyncProxy 
    {
      RequesterImpl * impl;
      DDS::SampleIdentity_t identity;

      SyncProxy(
        RequesterImpl * i,
        const DDS::SampleIdentity_t & sid)
        : impl(i),
          identity(sid)
      { }
    };

  public:
    /*
    RequesterImpl()
      : connext::Requester<TReq, TRep>(
          details::to_connext_requester_params(dds::rpc::RequesterParams())),
        sn(0),
        suppress_invalid(true)
    {}
    
    RequesterImpl(
        DDSDomainParticipant * participant,
        const std::string& service_name)
        : super(participant, service_name),
          service_name_(service_name),
          sn(0),
          suppress_invalid(true)
    {}
    */
    RequesterImpl(
        const dds::rpc::RequesterParams & params)
       :  service_name_(params.service_name()),
          connext::Requester<TReq, TRep>(
                details::to_connext_requester_params(params)),
          sn(0),
          suppress_invalid(true)
    { }

    void bind(const std::string & instance_name) override
    { }

    void unbind() override
    { }
    
    bool is_bound() const override
    { 
      return false;
    }
    
    std::string get_bound_instance_name() const override
    { 
      return "Unknown";
    }

    std::vector<std::string> get_discovered_service_instances() const override
    { 
      return std::vector<std::string>();
    }

    void wait_for_service() override
    { } 
    
    void wait_for_service(const dds::Duration & maxWait) override 
    { }

    void wait_for_service(std::string instanceName) override
    { }
    
    void wait_for_service(const dds::Duration & maxWait,
                          std::string instanceName) override
    { }

    void wait_for_services(int count) override
    { }
    
    void wait_for_services(const dds::Duration & maxWait, int count) override
    { }

    void wait_for_services(const std::vector<std::string> & instanceNames) override
    { }
    
    void wait_for_services(const dds::Duration & maxWait,
                           const std::vector<std::string> & instanceNames) override
    { }

    future<void> wait_for_service_async() override
    { 
      return future<void>();
    }
    
    future<void> wait_for_service_async(std::string instanceName) override
    { 
      return future<void>();
    }

    future<void> wait_for_services_async(int count) override
    { 
      return future<void>();
    }
    
    future<void> wait_for_services_async(
      const std::vector<std::string> & instanceNames) override
    { 
      return future<void>();
    }

    void close() override
    { }

    void send_request(WriteSampleRef<TReq> & wsref)
    {
      super::send_request(wsref);
    }

    bool receive_nondata_samples(bool enable)
    {
      bool old = suppress_invalid;
      suppress_invalid = enable;
      return old;
    }


    void send_request(TReq & req) 
    {
      DDS::WriteParams_t wparams;
      WriteSampleRef<TReq> wsref(req, wparams);
      //strcpy(req.header.serviceName, service_name_.c_str());
  
      if(instance_name_.size() > 0)
        strcpy(req.header.instanceName, instance_name_.c_str());
  
      req.header.requestId.sequence_number.low = ++sn;

      super::send_request(wsref);
      id2id_map[req.header.requestId] = wsref.identity();
    }

    bool receive_reply(
      Sample<TRep>& reply,
      const dds::SampleIdentity & relatedRequestId,
      const dds::Duration & timeout)
    {
      if (id2id_map.find(relatedRequestId) != id2id_map.end())
      {
        if (super::wait_for_replies(1, timeout))
        {            
          bool ret = super::take_reply(reply, id2id_map[relatedRequestId]);
          if (suppress_invalid && !reply.info().valid_data)
            return false;

          id2id_map.erase(relatedRequestId);
          return ret;
        }
        return false;
      }
      else
      {
        printf("Unknown dds::SampleIdentity\n");
        return false;
      }
    }

    static void * execute(void * arg)
    {
      //printf("Created thread = %d\n", RTIOsapiThread_getCurrentThreadID());
      
      std::unique_ptr<SyncProxy> sync(static_cast<SyncProxy *>(arg));

      if (sync->impl->wait_for_replies(
            1, dds::Duration::from_seconds(60), sync->identity))
      {
        try {
          Sample<TRep> reply;

          if (sync->impl->take_reply(reply, sync->identity))
          {
            if ((reply.data().header.relatedRequestId.sequence_number.low % 10) == 0)
              throw std::runtime_error("% 10 exception!");

            sync->impl->dict[sync->identity].set_value(reply);
          }
          else
          {
              sync->impl->dict[sync->identity].
                set_exception(
                  std::make_exception_ptr(
                    std::runtime_error("RequesterImpl::execute: Reply unavailable!")));
          }
        }
        catch (...) {
          sync->impl->dict[sync->identity].set_exception(std::current_exception());
        }
        sync->impl->dict.erase(sync->identity);
      }
      else
        printf("Request timed out\n");

      //printf("Future set. Thread complete\n");
      return NULL;
    }

    dds::rpc::future<Sample<TRep>> send_request_async(const TReq &req)
    {
      struct RTIOsapiThread* tid = 0;
      promise<Sample<TRep>> p;
      dds::rpc::future<Sample<TRep>> future = p.get_future();
      DDS::WriteParams_t wparams;
      WriteSampleRef<TReq> wsref(const_cast<TReq &>(req), wparams);

      //strcpy(req.header.serviceName, service_name_.c_str());

      if (instance_name_.size() > 0)
        strcpy(req.header.instanceName, instance_name_.c_str());

      const_cast<TReq &>(req).header.requestId.sequence_number.low = ++sn;

      super::send_request(wsref);
      SyncProxy * sync = new SyncProxy(this, wsref.identity());
      dict[wsref.identity()] = std::move(p);

      tid = RTIOsapiThread_new(
              "Reply Processor Thread", 
              RTI_OSAPI_THREAD_PRIORITY_NORMAL,
              RTI_OSAPI_THREAD_OPTION_DEFAULT,
              8192, // stack size
              NULL, // cpu bitmap
              execute, 
              sync);
      
      RTIOsapiThread_delete(tid);
      return future;
    }
};

template <class TReq, class TRep>
connext::ReplierParams<TReq, TRep>
  to_connext_replier_params(const rpc::ReplierParams & replier_params)
{
    return connext::ReplierParams<TReq, TRep>(
            replier_params.domain_participant())
            .service_name(replier_params.service_name());
}


template <class TReq, class TRep>
class ReplierImpl : public connext::Replier<TReq, TRep>,
                    public RPCEntityImpl
{
  private:
    std::string service_name_;
    std::string instance_name_;
    bool suppress_invalid;

    typedef connext::Replier<TReq, TRep> super;
  
  public:
/*    ReplierImpl(
        DDSDomainParticipant * participant,
        const std::string& service_name)
        : connext::Replier<TReq, TRep>(participant, service_name),
          service_name_(service_name),
          suppress_invalid(true)
    {}
*/
    ReplierImpl(
        const ReplierParams & params)
        : connext::Replier<TReq, TRep>(to_connext_replier_params<TReq, TRep>(params)),
          suppress_invalid(true)
    {
      service_name_ = params.service_name();
    }

	/*
    void send_reply_connext(
        TRep & reply,
        const Sample<TReq> & related_request_sample)
    {
      reply.header.relatedRequestId = 
        related_request_sample.data().header.requestId;
      super::send_reply(reply, related_request_sample.identity());
    }
    */

	void send_reply(
		TRep & reply,
		const dds::SampleIdentity & identity)
	{
		DDS::SampleIdentity_t connext_identity;
		memcpy(&connext_identity, &identity, sizeof(DDS::SampleIdentity_t));
		reply.header.relatedRequestId = identity;
		reply.header.remoteEx = REMOTE_EX_OK;
		super::send_reply(reply, connext_identity);
	}

    bool receive_request(Sample<TReq> & sample, const dds::Duration & timeout)
    {
      bool ret = super::receive_request(sample, timeout);
      if (suppress_invalid && !sample.info().valid_data)
        return false;
      
      return ret;
    }

    bool receive_nondata_samples(bool enable)
    {
      bool old = suppress_invalid;
      suppress_invalid = enable;
      return old; 
    }

    void close()
    { }
};

/****************************************************/
/************** RequesterParamsImpl *****************/
/****************************************************/

class RequesterParamsImpl
{
  DDSDomainParticipant * participant_;
  std::string service_name_;

public:
  RequesterParamsImpl();

  void domain_participant(DDSDomainParticipant *participant);
  void service_name(const std::string & service_name);

  DDSDomainParticipant *	domain_participant() const;
  std::string service_name() const;

};

/****************************************************/
/************** ReplierParamsImpl *****************/
/****************************************************/

class ReplierParamsImpl
{
  DDSDomainParticipant * participant_;
  std::string service_name_;

public:
  ReplierParamsImpl();

  void domain_participant(DDSDomainParticipant *participant);
  void service_name(const std::string & service_name);

  DDSDomainParticipant *	domain_participant() const;
  std::string service_name() const;

};

} // namespace details 

template <class Impl>
RPCEntity::RPCEntity(Impl impl, int)
: impl_(impl)
{}

template <class Impl>
ServiceProxy::ServiceProxy(Impl impl, int)
 : RPCEntity(impl, 0)
{}

/****************************************************/
/************** Requester ***************************/
/****************************************************/


template <typename TReq, typename TRep>
Requester<TReq, TRep>::Requester()
: ServiceProxy(new details::RequesterImpl<TReq, TRep>())
{ }

template <typename TReq, typename TRep>
Requester<TReq, TRep>::Requester(const RequesterParams& params)
: ServiceProxy(new details::RequesterImpl<TReq, TRep>(params), 0)
{ }

template <typename TReq, typename TRep>
Requester<TReq, TRep>::Requester(const Requester & requester)
    : ServiceProxy(requester)
{ }

template <typename TReq, typename TRep>
void Requester<TReq, TRep>::send_request(TReq & req)
{
  auto impl = dynamic_cast<details::RequesterImpl<TReq, TRep> *>(impl_.get());
  impl->send_request(req);
}

template <typename TReq, typename TRep>
bool Requester<TReq, TRep>::receive_reply(Sample<TRep> & sample, const dds::Duration & timeout)
{
  auto impl = static_cast<details::RequesterImpl<TReq, TRep> *>(impl_.get());
  return impl->receive_reply(sample, timeout);
}

template <class TReq, class TRep>
future<Sample<TRep>> Requester<TReq, TRep>::send_request_async(const TReq & req)
{
  auto impl = static_cast<details::RequesterImpl<TReq, TRep> *>(impl_.get());
  return impl->send_request_async(req);
}

template <class TReq, class TRep>
bool Requester<TReq, TRep>::wait_for_replies(const dds::Duration & max_wait)
{
  auto impl = static_cast<details::RequesterImpl<TReq, TRep> *>(impl_.get());
  return impl->wait_for_replies(max_wait);
}

#ifdef OMG_DDS_RPC_ENHANCED_PROFILE

template <class TReq, class TRep>
void Requester<TReq, TRep>::send_request(WriteSampleRef<TReq> & wsref)
{
  auto impl = static_cast<details::RequesterImpl<TReq, TRep> *>(impl_.get());
  impl->send_request(wsref);
}

#endif // OMG_DDS_RPC_ENHANCED_PROFILE

template <typename TReq, typename TRep>
bool Requester<TReq, TRep>::receive_nondata_samples(bool enable)
{
  auto impl = static_cast<details::RequesterImpl<TReq, TRep> *>(impl_.get());
  return impl->receive_nondata_samples(enable);
}

template <class TReq, class TRep>
bool Requester<TReq, TRep>::receive_reply(
    Sample<TRep>& reply,
    const dds::SampleIdentity & relatedRequestId, 
    const dds::Duration & timeout)
{
  auto impl = static_cast<details::RequesterImpl<TReq, TRep> *>(impl_.get());
  return impl->receive_reply(reply, relatedRequestId, timeout);
}

template <class TReq, class TRep>
typename Requester<TReq, TRep>::RequestDataWriter
Requester<TReq, TRep>::get_request_datawriter() const
{
  auto impl = static_cast<details::RequesterImpl<TReq, TRep> *>(impl_.get());
  return impl->get_request_datawriter();
}

template <class TReq, class TRep>
typename Requester<TReq, TRep>::ReplyDataReader
Requester<TReq, TRep>::get_reply_datareader() const
{
  auto impl = static_cast<details::RequesterImpl<TReq, TRep> *>(impl_.get());
  return impl->get_reply_datareader();
}


template <class TReq, class TRep>
Requester<TReq, TRep>::~Requester()
{ }

/**************************************************/
/************** Replier ***************************/
/**************************************************/


template <typename TReq, typename TRep>
Replier<TReq, TRep>::Replier()
: RPCEntity(boost::make_shared<details::ReplierImpl<TReq, TRep>>(), 0)
{ }

template <typename TReq, typename TRep>
Replier<TReq, TRep>::Replier(const ReplierParams& params)
: RPCEntity(boost::make_shared<details::ReplierImpl<TReq, TRep>>(params), 0)
{ }

template <typename TReq, typename TRep>
bool Replier<TReq, TRep>::receive_request(Sample<TReq> & sample, const dds::Duration & timeout)
{
  return static_cast<details::ReplierImpl<TReq, TRep> *>(impl_.get())->receive_request(sample, timeout);
}
/*
template <typename TReq, typename TRep>
void Replier<TReq, TRep>::send_reply_connext(
        TRep & reply,
        const Sample<TReq> & related_request_sample)
{
  impl_->send_reply_connext(reply, related_request_sample);
}
*/

template <typename TReq, typename TRep>
void Replier<TReq, TRep>::send_reply(
	TRep & reply,
	const dds::SampleIdentity & identity)
{
	static_cast<details::ReplierImpl<TReq, TRep> *>(impl_.get())->send_reply(reply, identity);
}

template <typename TReq, typename TRep>
bool Replier<TReq, TRep>::receive_nondata_samples(bool enable)
{
  return static_cast<details::ReplierImpl<TReq, TRep> *>(impl_.get())->receive_nondata_samples(enable);
}

template <class TReq, class TRep>
Replier<TReq, TRep>::~Replier()
{ }



} // namespace rpc
} //namespace dds

#endif // USE_RTI_CONNEXT
