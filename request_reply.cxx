#ifdef USE_RTI_CONNEXT

#include "connext_cpp/connext_cpp_requester.h"
#include "connext_cpp/connext_cpp_replier.h"

#include "boost/make_shared.hpp"

#include <map>

#ifdef RTI_WIN32
#define strcpy(dest, src) strcpy_s(dest, 255, src);
#endif 

bool operator < (
  const DDS::SampleIdentity_t & lhs,
  const DDS::SampleIdentity_t & rhs);

namespace dds {

  bool operator < (
    const dds::SampleIdentity_t & lhs,
    const dds::SampleIdentity_t & rhs);
}

namespace dds { namespace rpc {

#ifdef USE_PPLTASKS

using concurrency::task;

template <class ResultType>
shared_future<ResultType>::shared_future(future<ResultType> && fut)
  : shfut_(boost::make_shared<future<ResultType>>(std::move(fut)))
{}

namespace details
{

  template <typename RetType>
  struct Unwrapper
  {
    typedef future<RetType> return_type;

    template<typename Task, typename F>
    static return_type unwrap(Task & task, F&& func)
    {
      return task.then(std::forward<F>(func));
    }
  };

  template <typename T>
  struct Unwrapper<task<T>>
  {
    typedef future<T> return_type;

    template<typename Task, typename F>
    static return_type unwrap(Task & t, F&& func)
    {
      return t.then(std::forward<F>(func));
    }
  };

  template <typename T>
  struct Unwrapper<future<T>>
  {
    typedef future<T> return_type;

    template <typename Func, typename Task>
    class task_extractor
    {
      Func func;

    public:
      explicit task_extractor(Func && f)
        : func(std::move(f))
      {}

      explicit task_extractor(const Func & f)
        : func(f)
      {}

      template <class Arg>
      Task operator ()(Arg&& arg) const {
        return func(std::forward<Arg>(arg)).to_task();
      }
    };

    template<typename Task, typename F>
    static return_type unwrap(Task & t, F&& func)
    {
      return t.then(task_extractor<F, task<T>>(std::forward<F>(func)));
    }
  };

} // namespace details

template<class ResultType>
future<ResultType>::future()
  : task_() {}

template<class ResultType>
future<ResultType>::future(future<ResultType> && other)
  : task_(std::move(other.task_))
{ }

template<class ResultType>
future<ResultType> & future<ResultType>::operator =(future<ResultType> && other)
{
  task_ = std::move(other.task_);
  return *this;
}

template<class ResultType>
shared_future<ResultType> 
future<ResultType>::share()
{
  return std::move(*this);
}

#ifdef IMPLEMENTATION_DEPENDENT

template<class ResultType>
task<ResultType> future<ResultType>::to_task()
{
  return std::move(task_);
}

template<class ResultType>
future<ResultType>::future(const task<ResultType> & task)
: task_(task)
{}

template<class ResultType>
future<ResultType>::future(task<ResultType> && task)
: task_(std::move(task))
{}

#endif

template<class ResultType>
template<typename F>
typename details::Unwrapper<typename boost::result_of<F(future<ResultType> &&)>::type>::return_type
future<ResultType>::then(F&& func)
{
    typedef typename boost::result_of<F(future &&)>::type RetType;
    return details::Unwrapper<RetType>::unwrap(task_, std::move(func));
  }

template<class ResultType>
template<typename F>
typename details::Unwrapper<typename boost::result_of<F(future<ResultType> &&)>::type>::return_type
future<ResultType>::then(const F & func)
{
    typedef typename boost::result_of<F(future &&)>::type RetType;
    return details::Unwrapper<RetType>::unwrap(task_, func);
  }

/* // original implementation
template<typename F>
future<typename boost::result_of<F(future &&)>::type>
then(const F & func)
{
return task_.then(func);
}
*/

template<class ResultType>
void future<ResultType>::swap(future& other)
{
  using std::swap;
  swap(task_, other.task_);
}

template<class ResultType>
ResultType future<ResultType>::get()
{
  return task_.get();
}

template<class ResultType>
void future<ResultType>::wait() const
{
  task_.wait();
}

template<class ResultType>
bool future<ResultType>::is_ready() const
{
  return task_.is_done();
}

#endif // USE_PPLTASKS

namespace details 
{
#ifdef USE_BOOST_FUTURE

using boost::promise;

#endif

#ifdef USE_PPLTASKS

using concurrency::task_completion_event;

template <class ResultType>
class promise 
{
  task_completion_event<ResultType> tce_;

public:

  future<ResultType> get_future() const
  {
    return future<ResultType>(task<ResultType>(tce_));
  }

  void set_value(ResultType & result) const
  {
    tce_.set(result);
  }

  template <class Ex>
  bool set_exception(Ex ex) const
  {
    return tce_.set_exception(ex);
  }
};

#endif // USE_PPLTASKS

template <class TReq, class TRep>
class RequesterImpl : public connext::Requester<TReq, TRep>
{
  private:
    std::string service_name_;
    std::string instance_name_;
    long sn;
    bool suppress_invalid;
    std::map<DDS::SampleIdentity_t, promise<Sample<TRep>>> dict;
    std::map<dds::SampleIdentity_t, DDS::SampleIdentity_t> id2id_map;

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
    RequesterImpl(
        DDSDomainParticipant * participant,
        const std::string& service_name)
        : super(participant, service_name),
          service_name_(service_name),
          sn(0),
          suppress_invalid(true)
    {}

    RequesterImpl(
        const RequesterParams & params)
        : super(params),
          sn(0),
          suppress_invalid(true)
    {
      if(params.service_name())
        service_name_ = *params.service_name();
    }

    void send_request(WriteSampleRef<TReq> & wsref)
    {
      super::send_request(wsref);
    }

    void enable_nondata_samples()
    {
      suppress_invalid = false;
    }

    bool receive_reply(Sample<TRep> & reply, const DDS::SampleIdentity_t & relatedRequestId)
    {
      if (super::wait_for_replies(
            1,
            DDS::Duration_t::from_seconds(20),
            relatedRequestId))
      {
        return super::take_reply(reply, relatedRequestId);
      }
      else
        return false;
    }

    void send_request(TReq & req)
    {
      DDS::WriteParams_t wparams;
      WriteSampleRef<TReq> wsref(req, wparams);
      strcpy(req.header.serviceName, service_name_.c_str());
  
      if(instance_name_.size() > 0)
        strcpy(req.header.instanceName, instance_name_.c_str());
  
      req.header.requestId.seqnum.low = ++sn;

      super::send_request(wsref);
      id2id_map[req.header.requestId] = wsref.identity();
    }

    bool receive_reply(
      Sample<TRep>& reply,
      const dds::SampleIdentity_t & relatedRequestId)
    {
      if (id2id_map.find(relatedRequestId) != id2id_map.end())
      {
        if (super::wait_for_replies(
                    1,
                    DDS::Duration_t::from_seconds(20)))
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
        printf("Unknown dds::SampleIdentity_t\n");
        return false;
      }
    }

    static void * execute(void * arg)
    {
      //printf("Created thread = %d\n", RTIOsapiThread_getCurrentThreadID());
      
      std::unique_ptr<SyncProxy> sync(static_cast<SyncProxy *>(arg));

      if (sync->impl->wait_for_replies(
            1,
            DDS::Duration_t::from_seconds(60)))
      {
        Sample<TRep> reply;
        try {
          if (sync->impl->take_reply(reply, sync->identity))
          {
            if ((reply.data().header.relatedRequestId.seqnum.low % 5) == 0)
              throw std::runtime_error("% 5 exception!");

            sync->impl->dict[sync->identity].set_value(reply);
          }
          else
            printf("Reply unavailable\n");
        }
        catch (connext::UnsupportedException & ex) {
          sync->impl->dict[sync->identity].set_exception(ex);
        }
        catch (connext::BadParameterException & ex) {
          sync->impl->dict[sync->identity].set_exception(ex);
        }
        catch (connext::PreconditionNotMetException & ex) {
          sync->impl->dict[sync->identity].set_exception(ex);
        }
        catch (connext::ImmutablePolicyException & ex) {
          sync->impl->dict[sync->identity].set_exception(ex);
        }
        catch (connext::InconsistentPolicyException & ex) {
          sync->impl->dict[sync->identity].set_exception(ex);
        }
        catch (connext::NotEnabledException & ex) {
          sync->impl->dict[sync->identity].set_exception(ex);
        }
        catch (connext::AlreadyDeletedException & ex) {
          sync->impl->dict[sync->identity].set_exception(ex);
        }
        catch (connext::IllegalOperationException & ex) {
          sync->impl->dict[sync->identity].set_exception(ex);
        }
        catch (connext::TimeoutException & ex) {
          sync->impl->dict[sync->identity].set_exception(ex);
        }
        catch (connext::OutOfResourcesException & ex) {
          sync->impl->dict[sync->identity].set_exception(ex);
        }
        catch (std::runtime_error & ex) {
          sync->impl->dict[sync->identity].set_exception(ex);
        }
        catch (std::exception & ex) {
          sync->impl->dict[sync->identity].set_exception(ex);
        }
        sync->impl->dict.erase(sync->identity);
      }
      else
        printf("Request timed out\n");

      //printf("Future set. Thread complete\n");
      return NULL;
    }

    future<Sample<TRep>> send_request_async(TReq &req)
    {
      struct RTIOsapiThread* tid = 0;
      promise<Sample<TRep>> p;
      future<Sample<TRep>> future = p.get_future();
      DDS::WriteParams_t wparams;
      WriteSampleRef<TReq> wsref(req, wparams);

      strcpy(req.header.serviceName, service_name_.c_str());

      if (instance_name_.size() > 0)
        strcpy(req.header.instanceName, instance_name_.c_str());

      req.header.requestId.seqnum.low = ++sn;

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
class ReplierImpl : public connext::Replier<TReq, TRep>
{
  private:
    std::string service_name_;
    std::string instance_name_;
    bool suppress_invalid;

    typedef connext::Replier<TReq, TRep> super;
  
  public:
    ReplierImpl(
        DDSDomainParticipant * participant,
        const std::string& service_name)
        : connext::Replier<TReq, TRep>(participant, service_name),
          service_name_(service_name),
          suppress_invalid(true)
    {}

    ReplierImpl(
        const ReplierParams & params)
        : connext::Replier<TReq, TRep>(params),
          suppress_invalid(true)
    {
      if(params.service_name())
        service_name_ = *params.service_name();
    }

    void send_reply_connext(
        TRep & reply,
        const Sample<TReq> & related_request_sample)
    {
      reply.header.relatedRequestId = related_request_sample.data().header.requestId;
      super::send_reply(reply, related_request_sample.identity());
    }
    
    bool receive_request(Sample<TReq> & sample, const DDS::Duration_t & timeout)
    {
      bool ret = super::receive_request(sample, timeout);
      if (suppress_invalid && !sample.info().valid_data)
        return false;
      
      return ret;
    }

    void enable_nondata_samples()
    {
      suppress_invalid = false;
    }
};

} // namespace details 


template <typename TReq, typename TRep>
Requester<TReq, TRep>::Requester(
        DDSDomainParticipant * participant,
        const std::string& service_name)
        : impl_(boost::make_shared<details::RequesterImpl<TReq, TRep>>(participant, service_name))
{ }

template <typename TReq, typename TRep>
Requester<TReq, TRep>::Requester(const RequesterParams& params)
  : impl_(boost::make_shared<details::RequesterImpl<TReq, TRep>>(params))
{ }

template <typename TReq, typename TRep>
Requester<TReq, TRep>::Requester(const Requester & requester)
  : impl_(requester.impl_)
{ }

template <typename TReq, typename TRep>
Requester<TReq, TRep> & Requester<TReq, TRep>::operator = (const Requester & requester)
{ 
  this->impl_ = requester.impl_;
  return *this;
}

template <typename TReq, typename TRep>
Requester<TReq, TRep>::~Requester()
{}

template <typename TReq, typename TRep>
void Requester<TReq, TRep>::send_request(TReq & req)
{
  impl_->send_request(req);
}

template <typename TReq, typename TRep>
bool Requester<TReq, TRep>::receive_reply(Sample<TRep> & sample, const DDS_Duration_t & timeout)
{
  return impl_->receive_reply(sample, timeout);
}

template <class TReq, class TRep>
future<Sample<TRep>> Requester<TReq, TRep>::send_request_async(TReq & req)
{
  return impl_->send_request_async(req);
}

template <class TReq, class TRep>
bool Requester<TReq, TRep>::wait_for_replies(const DDS_Duration_t& max_wait)
{
  return impl_->wait_for_replies(max_wait);
}

#ifdef OMG_DDS_RPC_ENHANCED_PROFILE

template <class TReq, class TRep>
void Requester<TReq, TRep>::send_request(WriteSampleRef<TReq> & wsref)
{
  impl_->send_request(wsref);
}

#endif // OMG_DDS_RPC_ENHANCED_PROFILE

template <class TReq, class TRep>
bool Requester<TReq, TRep>::receive_reply_connext(
  Sample<TRep> & reply, 
  const DDS::SampleIdentity_t & relatedRequestId)
{
  return impl_->receive_reply(reply, relatedRequestId);
}


template <typename TReq, typename TRep>
void Requester<TReq, TRep>::enable_nondata_samples()
{
  impl_->enable_nondata_samples();
}

template <class TReq, class TRep>
bool Requester<TReq, TRep>::receive_reply(
    Sample<TRep>& reply,
    const dds::SampleIdentity_t & relatedRequestId)
{
  return impl_->receive_reply(reply, relatedRequestId);
}







template <typename TReq, typename TRep>
Replier<TReq, TRep>::Replier(
        DDSDomainParticipant * participant,
        const std::string& service_name)
        : impl_(boost::make_shared<details::ReplierImpl<TReq, TRep>>(participant, service_name))
{ }

template <typename TReq, typename TRep>
Replier<TReq, TRep>::Replier(
        DDSDomainParticipant * participant,
        const std::string& service_name,
        const std::string& instance_name)
        : impl_(boost::make_shared<details::ReplierImpl<TReq, TRep>>(participant, service_name, instance_name))
{ }

template <typename TReq, typename TRep>
Replier<TReq, TRep>::Replier(const ReplierParams& params)
  : impl_(boost::make_shared<details::ReplierImpl<TReq, TRep>>(params))
{ }


template <typename TReq, typename TRep>
Replier<TReq, TRep> & Replier<TReq, TRep>::operator = (const Replier & replier)
{ 
  this->impl_ = replier.impl_;
  return *this;
}

template <typename TReq, typename TRep>
Replier<TReq, TRep>::Replier(const Replier& replier)
  : impl_(replier.impl_)
{ }


template <typename TReq, typename TRep>
Replier<TReq, TRep>::~Replier()
{}

template <typename TReq, typename TRep>
bool Replier<TReq, TRep>::receive_request(Sample<TReq> & sample, const DDS::Duration_t & timeout)
{
  return impl_->receive_request(sample, timeout);
}

template <typename TReq, typename TRep>
void Replier<TReq, TRep>::send_reply_connext(
        TRep & reply,
        const Sample<TReq> & related_request_sample)
{
  impl_->send_reply_connext(reply, related_request_sample);
}

template <typename TReq, typename TRep>
void Replier<TReq, TRep>::enable_nondata_samples()
{
  impl_->enable_nondata_samples();
}



} // namespace rpc
} //namespace dds

#endif // USE_RTI_CONNEXT
