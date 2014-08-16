#ifndef OMG_DDS_RPC_REQUEST_REPLY_H
#define OMG_DDS_RPC_REQUEST_REPLY_H

#include <string>

#ifdef USE_BOOST_FUTURE
#define BOOST_THREAD_PROVIDES_FUTURE
#define BOOST_THREAD_PROVIDES_FUTURE_CONTINUATION
#define BOOST_RESULT_OF_USE_DECLTYPE
#include "boost/thread/future.hpp"
#endif

#ifdef USE_PPLTASKS

#include <ppltasks.h>

#define BOOST_RESULT_OF_USE_DECLTYPE
#include "boost/utility/result_of.hpp"

#endif

#include "sample.h"
#include "rpc_types.h"

#include "boost/shared_ptr.hpp"
#include "ndds/ndds_namespace_cpp.h"

namespace dds {
  namespace rpc {

#ifdef IMPLEMENTATION_DEPENDENT
    namespace details {
      template <class, class>
      class RequesterImpl;

      template <class, class>
      class ReplierImpl;
      
      template <class T>
      struct Unwrapper;
    } // namespace details
#endif

    class ReplierParams;
    class RequesterParams;

#ifdef USE_BOOST_FUTURE

using boost::future;

#endif 

#ifdef USE_PPLTASKS

#ifdef IMPLEMENTATION_DEPENDENT
using concurrency::task;
#endif

template <class R>
class future;

template <class ResultType>
class shared_future
{
#ifdef IMPLEMENTATION_DEPENDENT
  boost::shared_ptr<future<ResultType>> shfut_;
#endif 

public:

  explicit shared_future(future<ResultType> && fut);

  explicit shared_future(future<ResultType> & fut);
};

template <class ResultType>
class future
{
#ifdef IMPLEMENTATION_DEPENDENT
  task<ResultType> task_;
#endif

public:

  typedef ResultType value_type;
  
  future();
  future(future && other);
  future& operator=(future && other);

  future(const future & rhs) = delete;
  future& operator=(const future & rhs) = delete;
  
#ifdef IMPLEMENTATION_DEPENDENT

  task<ResultType> to_task();
  future(const task<ResultType> & task);
  future(task<ResultType> && task);

#endif

  template<typename F>
  typename details::Unwrapper<typename boost::result_of<F(future &&)>::type>::return_type
    then(F&& func);

  template<typename F>
  typename details::Unwrapper<typename boost::result_of<F(future &&)>::type>::return_type
    then(const F & func);


  shared_future<ResultType> share();
  void swap(future& other);
  ResultType get();
  bool has_exception() const;
  bool has_value() const;
  bool is_ready() const; 
  
  void wait() const;

  template <class Duration>
  void wait(const Duration &);
};


#endif // USE_PPLTASKS

using boost::shared_ptr;

template <class T>
struct rpc_type_traits 
#ifdef IMPLEMENTATION_DEPENDENT
  : public connext::dds_type_traits<T>
#endif
{
  // typedef DataWriter        for T
  // typedef DataReader        for T
  // typedef LoanedSamplesType for T.
};

template <typename TReq, typename TRep>
class Requester
{
public:

    typedef TReq RequestType;

    typedef TRep ReplyType;

    typedef typename rpc_type_traits<TReq>::DataWriter RequestDataWriter;

    typedef typename rpc_type_traits<TRep>::DataReader ReplyDataReader;

    typedef typename rpc_type_traits<TRep>::LoanedSamplesType LoanedSamplesType;

    typedef RequesterParams Params;

    Requester(
        DDS::DomainParticipant * participant,
        const std::string& service_name);

    explicit Requester(const RequesterParams& params);

    Requester (const Requester &);

    void swap(Requester & other);

    Requester & operator = (const Requester &);

    virtual ~Requester();

#ifdef OMG_DDS_RPC_BASIC_PROFILE
    void send_request(TReq & request);
    void send_request_oneway(TReq &);
    future<Sample<TRep>> send_request_async(TReq &);
#endif 

#ifdef OMG_DDS_RPC_ENHANCED_PROFILE
    void send_request(const TReq & request);
    void send_request_oneway(const TReq &);
    future<Sample<TRep>> send_request_async(const TReq &);
#endif 

#ifdef OMG_DDS_RPC_ENHANCED_PROFILE
    void send_request(WriteSample<TReq> &request)
    void send_request(WriteSampleRef<TReq> & wsref);
#endif

    bool receive_reply(
      Sample<TRep>& reply,
      const DDS_Duration_t& timeout);

    bool receive_reply(
      SampleRef<TRep> reply,
      const DDS_Duration_t& timeout);

    bool receive_reply(
      Sample<TRep>& reply,
      const SampleIdentity_t & relatedRequestId);

    bool receive_reply(
      SampleRef<TRep> reply,
      const SampleIdentity_t & relatedRequestId);

    LoanedSamples<TRep> receive_replies(const DDS_Duration_t& max_wait);

    LoanedSamples<TRep> receive_replies(int min_count,
                                        int max_count,
                                        const DDS_Duration_t & max_wait);

    bool wait_for_replies(
        int min_count,
        const DDS_Duration_t& max_wait);

    bool wait_for_replies(const DDS_Duration_t& max_wait);

    bool wait_for_replies(
        int min_count,
        const DDS_Duration_t& max_wait,
        const SampleIdentity_t & related_request_id);

    bool take_reply(Sample<TRep>& reply);

    bool take_reply(SampleRef<TRep> reply);

    bool take_reply(
        Sample<TRep>& reply, 
        const SampleIdentity_t& related_request_id);

    bool take_reply(
        SampleRef<TRep> reply, 
        const SampleIdentity_t& related_request_id);

    LoanedSamples<TRep> take_replies(int max_count);

    LoanedSamples<TRep> take_replies(
        int max_count, 
        const SampleIdentity_t& related_request_id);

    LoanedSamples<TRep> take_replies(
        const SampleIdentity_t& related_request_id);

    inline bool read_reply(Sample<TRep>& reply);

    inline bool read_reply(SampleRef<TRep> reply);

    inline bool read_reply(
        Sample<TRep>& reply, 
        const SampleIdentity_t& related_request_id);

    inline bool read_reply(
        SampleRef<TRep> reply, 
        const SampleIdentity_t& related_request_id);

    LoanedSamples<TRep> read_replies(int max_count);

    LoanedSamples<TRep> read_replies(
        int max_count, 
        const SampleIdentity_t& related_request_id);

    LoanedSamples<TRep> read_replies(
        const SampleIdentity_t& related_request_id);

    RequestDataWriter * get_request_datawriter() const;

    ReplyDataReader * get_reply_datareader() const;

    RequesterParams get_params() const;

    void set_datawriter_qos(const DDS::DataWriterQos & dwqos);

    void set_datareader_qos(const DDS::DataReaderQos & drqos);
    
    void wait_for_replier(
      unsigned howmany, 
      const DDS::Duration_t &);

    void wait_for_replier(
      const std::string & instance_name, 
      const DDS::Duration_t &);

    bool bind(const std::string & instance_name);
    
    bool unbind();
    
    bool is_bound() const;

    const std::string & bound_instance() const;

    void enable_nondata_samples();

#ifdef IMPLEMENTATION_DEPENDENT
public:
    typedef typename dds_type_traits<TReq>::TypeSupport RequestTypeSupport;
    typedef typename dds_type_traits<TRep>::TypeSupport ReplyTypeSupport;
    typedef shared_ptr<details::RequesterImpl<TReq, TRep>> ImplDependent;

    bool receive_reply_connext(
      Sample<TRep> & reply,
      const DDS::SampleIdentity_t & relatedRequestId);

private:
  shared_ptr<details::RequesterImpl<TReq, TRep>> impl_;

#endif // IMPLEMENTATION_DEPENDENT

  ImplDependent get_impl();

};

template <typename TReq, typename TRep>
class Replier
{
  public:

    typedef TReq RequestType;

    typedef TRep ReplyType;

    typedef typename rpc_type_traits<TRep>::DataWriter ReplierDataWriter;

    typedef typename rpc_type_traits<TReq>::DataReader RequestDataReader;

    typedef typename rpc_type_traits<TReq>::LoanedSamplesType LoanedSamplesType;

    typedef ReplierParams Params;

    Replier(DDSDomainParticipant * participant,
            const std::string & service_name);

    Replier(DDSDomainParticipant * participant,
            const std::string & service_name,
            const std::string & instance_name);

    Replier(const Replier &);

    Replier & operator = (const Replier &);

    explicit Replier(const ReplierParams & params);
    
    virtual ~Replier();

    void swap(Replier & other);
    
    void send_reply(
        const TRep & reply,
        const SampleIdentity_t& related_request_id);

    bool receive_request(
        Sample<TReq> & request,
        const DDS::Duration_t & max_wait);

    bool receive_request(
        SampleRef<TReq> request,
        const DDS::Duration_t & max_wait);

    LoanedSamplesType receive_requests(
        const DDS::Duration_t & max_wait);

    LoanedSamplesType receive_requests(
        int min_request_count,
        int max_request_count,
        const DDS::Duration_t& max_wait);

    bool wait_for_requests(
        const DDS::Duration_t & max_wait);

    bool wait_for_requests(
        int min_count, 
        const DDS::Duration_t & max_wait);

    bool take_request(Sample<TReq> & request);

    bool take_request(SampleRef<TReq> request);

    LoanedSamplesType take_requests(int max_samples);

    bool read_request(Sample<TReq> & request);

    bool read_request(SampleRef<TReq> request);

    LoanedSamplesType read_requests(int max_samples);

    RequestDataReader * get_request_datareader() const;

    ReplierDataWriter * get_reply_datawriter() const;

    const ReplierParams & get_params() const;

    void set_datawriter_qos(const DDS::DataWriterQos & dwqos);

    void set_datareader_qos(const DDS::DataReaderQos & drqos);

    void enable_nondata_samples();

#ifdef IMPLEMENTATION_DEPENDENT
public:
  typedef typename dds_type_traits<TReq>::TypeSupport RequestTypeSupport;
  typedef typename dds_type_traits<TRep>::TypeSupport ReplyTypeSupport;
  typedef shared_ptr<details::ReplierImpl<TReq, TRep>> ImplDependent;

  void send_reply_connext(
    TRep & reply,
    const Sample<TReq> & related_request_sample);

private:
  shared_ptr<details::ReplierImpl<TReq, TRep>> impl_;

#endif // IMPLEMENTATION_DEPENDENT

  ImplDependent get_impl();

};

template <class TReq, class TRep>
void swap(Replier<TReq, TRep> & cr1,  
          Replier<TReq, TRep> & cr2)
{
    cr1.swap(cr2);
}

class ListenerBase 
{
  virtual ~ListenerBase();
};

template <class TReq, class TRep>
class SimpleReplierListener : public ListenerBase
{
public:
  virtual TRep * process_request(const Sample<TReq> &, 
                                 const SampleIdentity_t &) = 0;
  virtual ~SimpleReplierListener();
};

template <class TReq, class TRep>
class ReplierListener : public ListenerBase
{
public:
  virtual void on_request_available(Replier<TReq, TRep> &) = 0;
  virtual ~ReplierListener();
};

template <class TRep>
class SimpleRequesterListener : public ListenerBase
{
public:
  virtual void process_reply(const Sample<TRep> &, 
                             const SampleIdentity_t &) = 0;
  virtual ~SimpleRequesterListener();
};

template <class TReq, class TRep>
class RequesterListener : public ListenerBase
{
public:
  virtual void on_reply_available(Requester<TReq, TRep> &) = 0;
  virtual ~RequesterListener();
};

class RequesterParams 
{
public:
    RequesterParams ();
   
    template <class TRep>
    RequesterParams & 	simple_requester_listener(
        SimpleRequesterListener<TRep> *listener);

    template <class TReq, class TRep>
    RequesterParams & 	requester_listener(
        RequesterListener<TReq, TRep> *listener);

    RequesterParams & 	domain_participant(DDS::DomainParticipant *participant);
    RequesterParams & 	service_name (const std::string &name);
    RequesterParams & 	request_topic_name (const std::string &name);
    RequesterParams & 	reply_topic_name (const std::string &name);
    RequesterParams & 	datawriter_qos (const DDS::DataWriterQos &qos);
    RequesterParams & 	datareader_qos (const DDS::DataReaderQos &qos);
    RequesterParams & 	publisher (DDS::Publisher *publisher);
    RequesterParams & 	subscriber (DDS::Subscriber *subscriber);

    DDS::DomainParticipant * domain_participant() const;
    ListenerBase * simple_requester_listener() const;
    ListenerBase * requester_listener() const;
    const std::string * service_name() const;
    const std::string * request_topic_name() const;
    const std::string * reply_topic_name() const;
    const DDS::DataWriterQos * datawriter_qos() const;
    const DDS::DataReaderQos * datareader_qos() const;
    DDS::Publisher * publisher() const;
    DDS::Subscriber * subscriber() const;
};     


class ReplierParams 
{
public:
    ReplierParams ();

    template <class TReq, class TRep>
    ReplierParams & simple_replier_listener (
        SimpleReplierListener<TReq, TRep> *listener);    
    
    template <class TReq, class TRep>
    ReplierParams & replier_listener (
        ReplierListener<TReq, TRep> *listener);
    
    ReplierParams & domain_participant(DDS::DomainParticipant *participant);
    ReplierParams & service_name (const std::string &service_name);
    ReplierParams & instance_name (const std::string &service_name);
    ReplierParams & request_topic_name (const std::string &req_topic);
    ReplierParams & reply_topic_name (const std::string &rep_topic);
    ReplierParams & datawriter_qos (const DDS::DataWriterQos &qos);
    ReplierParams & datareader_qos (const DDS::DataReaderQos &qos);
    ReplierParams & publisher (DDS::Publisher *publisher);
    ReplierParams & subscriber (DDS::Subscriber *subscriber);

    DDS::DomainParticipant * domain_participant() const;
    ListenerBase * simple_replier_listener() const;
    ListenerBase * replier_listener() const;
    const std::string * service_name() const;
    const std::string * instance_name() const;
    const std::string * request_topic_name() const;
    const std::string * reply_topic_name() const;
    const DDS::DataWriterQos * datawriter_qos() const;
    const DDS::DataReaderQos * datareader_qos() const;
    DDS::Publisher * publisher() const;
    DDS::Subscriber * subscriber() const;
};

} // namespace rpc

} // namespace dds 

#include "request_reply.cxx"

#endif // OMG_DDS_RPC_REQUEST_REPLY_H

