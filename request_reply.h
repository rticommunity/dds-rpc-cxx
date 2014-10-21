#ifndef OMG_DDS_RPC_REQUEST_REPLY_H
#define OMG_DDS_RPC_REQUEST_REPLY_H

#include <string> // platform-specific

#include "vendor_dependent.h"
#include "sample.h"   // standard

namespace dds {

namespace rpc {

class ServiceProxy
{
public:

  template <class Impl>
  ServiceProxy(Impl impl);

  void bind(const std::string & instance_name);
  void unbind();
  bool is_bound() const;
  std::string get_bound_instance_name() const;
  std::vector<std::string> get_discoverd_service_instances() const;

  void wait_for_service();
  void wait_for_service(const dds::Duration_t & maxWait);

  void wait_for_service(std::string instanceName);
  void wait_for_service(const dds::Duration_t & maxWait,
                        std::string instanceName);

  void wait_for_services(int count);
  void wait_for_services(const dds::Duration_t & maxWait, int count);

  void wait_for_services(const std::vector<std::string> & instanceNames);
  void wait_for_services(const dds::Duration_t & maxWait,
                         const std::vector<std::string> & instanceNames);

  future<void> wait_for_service_async();
  future<void> wait_for_service_async(std::string instanceName);
  future<void> wait_for_services_async(int count);
  future<void> wait_for_services_async(const std::vector<std::string> & instanceNames);

  void close();

protected:
  typedef details::vendor_dependent<ServiceProxy>::type VendorDependent;
  VendorDependent impl_;

public:
  VendorDependent get_impl() const;
};


template <typename TReq, typename TRep>
class Requester : public ServiceProxy
{
public:

    typedef TReq RequestType;

    typedef TRep ReplyType;

    typedef typename dds::dds_type_traits<TReq>::DataWriter RequestDataWriter;

    typedef typename dds::dds_type_traits<TRep>::DataReader ReplyDataReader;

    typedef typename dds::dds_type_traits<TRep>::LoanedSamplesType LoanedSamplesType;

    typedef RequesterParams Params;

    Requester();
    
    explicit Requester(const RequesterParams& params);

    Requester (const Requester &);

    void swap(Requester & other);

    Requester & operator = (const Requester &);

    virtual ~Requester();

#ifdef OMG_DDS_RPC_BASIC_PROFILE
    void send_request(TReq & request);
    void send_request_oneway(TReq &);
    future<dds::Sample<TRep>> send_request_async(TReq &);
#endif 

#ifdef OMG_DDS_RPC_ENHANCED_PROFILE
    void send_request(WriteSample<TReq> &request)
    void send_request(WriteSampleRef<TReq> & wsref);
    void send_request(const TReq & request);
    void send_request_oneway(const TReq &);

    future<Sample<TRep>> send_request_async(const TReq &);
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

    bool receive_nondata_samples(bool enable);

    RequesterParams get_requester_params() const;

    RequestDataWriter get_request_datawriter() const;

    ReplyDataReader get_reply_datareader() const;

private:
    typedef typename details::vendor_dependent<Requester<TReq, TRep>>::type VendorDependent;
    VendorDependent impl_;

public:
    VendorDependent get_impl();
};

template <typename TReq, typename TRep>
class Replier 
{
  public:

    typedef TReq RequestType;

    typedef TRep ReplyType;

    typedef typename dds::dds_type_traits<TRep>::DataWriter ReplyDataWriter;

    typedef typename dds::dds_type_traits<TReq>::DataReader RequestDataReader;

    typedef typename dds::dds_type_traits<TReq>::LoanedSamplesType LoanedSamplesType;

    typedef ReplierParams Params;

    Replier();

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
        const dds::Duration_t & max_wait);

    bool receive_request(
        SampleRef<TReq> request,
        const dds::Duration_t & max_wait);

    LoanedSamplesType receive_requests(
        const dds::Duration_t & max_wait);

    LoanedSamplesType receive_requests(
        int min_request_count,
        int max_request_count,
        const dds::Duration_t& max_wait);

    bool wait_for_requests(
        const dds::Duration_t & max_wait);

    bool wait_for_requests(
        int min_count, 
        const dds::Duration_t & max_wait);

    bool take_request(Sample<TReq> & request);

    bool take_request(SampleRef<TReq> request);

    LoanedSamplesType take_requests(int max_samples);

    bool read_request(Sample<TReq> & request);

    bool read_request(SampleRef<TReq> request);

    LoanedSamplesType read_requests(int max_samples);

    const ReplierParams & get_replier_params() const;

    bool receive_nondata_samples(bool enable);

    RequestDataReader get_request_datareader() const;

    ReplyDataWriter get_reply_datawriter() const;

    void close();

private:
  typedef typename details::vendor_dependent<Replier<TReq, TRep>>::type VendorDependent;
  VendorDependent impl_;

public:
  VendorDependent get_impl();
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
    RequesterParams(const RequesterParams & other);
    RequesterParams & operator = (const RequesterParams & that);

    template <class TRep>
    RequesterParams & 	simple_requester_listener(
        SimpleRequesterListener<TRep> *listener);

    template <class TReq, class TRep>
    RequesterParams & 	requester_listener(
        RequesterListener<TReq, TRep> *listener);

    RequesterParams & 	domain_participant(dds::DomainParticipant * participant);
    RequesterParams & 	publisher(dds::Publisher * publisher);
    RequesterParams & 	subscriber(dds::Subscriber * subscriber);
    RequesterParams & 	datawriter_qos(const dds::DataWriterQos & qos);
    RequesterParams & 	datareader_qos(const dds::DataReaderQos & qos);
    RequesterParams & 	service_name (const std::string &name);
    RequesterParams & 	request_topic_name (const std::string &name);
    RequesterParams & 	reply_topic_name (const std::string &name);

    dds::DomainParticipant * domain_participant() const;
    dds::Publisher * publisher() const;
    dds::Subscriber * subscriber() const;
    const dds::DataWriterQos & datawriter_qos() const;
    const dds::DataReaderQos & datareader_qos() const;
    ListenerBase * simple_requester_listener() const;
    ListenerBase * requester_listener() const;
    std::string service_name() const;
    std::string request_topic_name() const;
    std::string reply_topic_name() const;

private:
    typedef details::vendor_dependent<RequesterParams>::type VendorDependent;
    VendorDependent impl_;

public:
    VendorDependent get_impl();
};


class ReplierParams 
{
public:
    ReplierParams ();
    ReplierParams(const ReplierParams & other);
    ReplierParams & operator = (const ReplierParams & that);

    template <class TReq, class TRep>
    ReplierParams & simple_replier_listener (
        SimpleReplierListener<TReq, TRep> *listener);    
    
    template <class TReq, class TRep>
    ReplierParams & replier_listener (
        ReplierListener<TReq, TRep> *listener);
    
    ReplierParams & domain_participant(dds::DomainParticipant *participant);
    ReplierParams & service_name (const std::string &service_name);
    ReplierParams & instance_name (const std::string &service_name);
    ReplierParams & request_topic_name (const std::string &req_topic);
    ReplierParams & reply_topic_name (const std::string &rep_topic);
    ReplierParams & datawriter_qos (const dds::DataWriterQos &qos);
    ReplierParams & datareader_qos (const dds::DataReaderQos &qos);
    ReplierParams & publisher (dds::Publisher *publisher);
    ReplierParams & subscriber (dds::Subscriber *subscriber);

    dds::DomainParticipant * domain_participant() const;
    ListenerBase * simple_replier_listener() const;
    ListenerBase * replier_listener() const;
    std::string service_name() const;
    std::string instance_name() const;
    std::string request_topic_name() const;
    std::string reply_topic_name() const;
    const dds::DataWriterQos * datawriter_qos() const;
    const dds::DataReaderQos * datareader_qos() const;
    dds::Publisher * publisher() const;
    dds::Subscriber * subscriber() const;

private:
  typedef details::vendor_dependent<ReplierParams>::type VendorDependent;
  VendorDependent impl_;

public:
  VendorDependent get_impl();
};

} // namespace rpc

} // namespace dds 

#include "request_reply.hpp" // vendor dependent

#endif // OMG_DDS_RPC_REQUEST_REPLY_H

