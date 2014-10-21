#ifndef VENDOR_DEPENDENT_H
#define VENDOR_DEPENDENT_H

struct DDS_Duration_t;
struct DDS_SampleInfo;
struct DDS_SampleInfoSeq;
struct DDS_DataReaderQos;
struct DDS_DataWriterQos;
struct DDS_SampleIdentity_t;

class DDSDomainParticipant;
class DDSPublisher;
class DDSSubscriber;

namespace boost {

  template <class T>
  class shared_ptr;

} // namespace boost

namespace connext {

  template <class T>
  class Sample;

  template <class T>
  class SampleRef;

  template <class T>
  class WriteSample;

  template <class T>
  class WriteSampleRef;

  template <class T>
  class LoanedSamples;

  template <class T, bool>
  class SampleIterator;

  template <class T>
  struct dds_type_traits;

} // namespace connext

namespace dds {

  typedef ::DDS_Duration_t            Duration_t;
  typedef ::DDS_SampleInfo            SampleInfo;
  typedef ::DDS_SampleInfoSeq         SampleInfoSeq;
  typedef ::DDS_DataReaderQos         DataReaderQos;
  typedef ::DDS_DataWriterQos         DataWriterQos;
  
  
  typedef ::DDSDomainParticipant      DomainParticipant;
  typedef ::DDSPublisher              Publisher;
  typedef ::DDSSubscriber             Subscriber;

  using connext::Sample;
  using connext::SampleRef;
  using connext::WriteSample;
  using connext::WriteSampleRef;
  using connext::LoanedSamples;
  using connext::SampleIterator;
  using connext::dds_type_traits;

  template <class T>
  class SharedSamples;

  namespace rpc {

    class ServiceProxy;
    class RequesterParams;
    class ReplierParams;
    class ServiceParams;
    class ClientParams;
    class ServerParams;
    class ServiceEndpoint;
    class Server;

    template <class TReq, class TRep>
    class Requester;

    template <class TReq, class TRep>
    class Replier;

    template <class R>
    class shared_future;

    namespace details {

      class ServerImpl;
      class ServiceParamsImpl;
      class ClientParamsImpl;
      class ServerParamsImpl;
      class ServiceEndpointImpl;
      class RequesterParamsImpl;
      class ReplierParamsImpl;
      class ServiceProxyImpl;

      template <class, class>
      class RequesterImpl;

      template <class Iface>
      class ClientImpl;

      template <class, class>
      class ReplierImpl;

      template <class T>
      struct Unwrapper;

      template <class T>
      struct vendor_dependent;

      template <>
      struct vendor_dependent<dds::rpc::ServiceProxy>
      {
        typedef boost::shared_ptr<details::ServiceProxyImpl> type;
      };

      template <>
      struct vendor_dependent<dds::rpc::RequesterParams>
      {
        typedef boost::shared_ptr<details::RequesterParamsImpl> type;
      };

      template <>
      struct vendor_dependent<dds::rpc::ReplierParams>
      {
        typedef boost::shared_ptr<details::ReplierParamsImpl> type;
      };

      template <>
      struct vendor_dependent<dds::rpc::ServiceParams>
      {
        typedef boost::shared_ptr<details::ServiceParamsImpl> type;
      };

      template <>
      struct vendor_dependent<dds::rpc::ClientParams>
      {
        typedef boost::shared_ptr<details::ClientParamsImpl> type;
      };

      template <>
      struct vendor_dependent<dds::rpc::ServerParams>
      {
        typedef boost::shared_ptr<details::ServerParamsImpl> type;
      };

      template <>
      struct vendor_dependent<dds::rpc::ServiceEndpoint>
      {
        typedef boost::shared_ptr<details::ServiceEndpointImpl> type;
      };

      template <>
      struct vendor_dependent<dds::rpc::Server>
      {
        typedef boost::shared_ptr<details::ServerImpl> type;
      };

      template <class TReq, class TRep>
      struct vendor_dependent<dds::rpc::Requester<TReq, TRep>>
      {
        typedef boost::shared_ptr<details::RequesterImpl<TReq, TRep>> type;
      };

      template <class TReq, class TRep>
      struct vendor_dependent<dds::rpc::Replier<TReq, TRep>>
      {
        typedef boost::shared_ptr<details::ReplierImpl<TReq, TRep>> type;
      };

    } // namespace details 
  } // namespace rpc
} // namespace dds



#include "rpc_types.h"  
#include "boost/shared_ptr.hpp"
#include "ndds/ndds_requestreply_cpp.h"


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

namespace dds {
  namespace rpc { 

#ifdef USE_BOOST_FUTURE

// Pull future<T> from boost into this namespace.
using boost::future;

#endif 

#ifdef USE_PPLTASKS

using concurrency::task;

template <class R>
class future;

template <class ResultType>
class shared_future
{
  boost::shared_ptr<future<ResultType>> shfut_;

public:

  explicit shared_future(future<ResultType> && fut);

  explicit shared_future(future<ResultType> & fut);
};

template <class ResultType>
class future
{
  task<ResultType> task_;

public:

  typedef ResultType value_type;

  future();
  future(future && other);
  future& operator=(future && other);

  future(const future & rhs) = delete;
  future& operator=(const future & rhs) = delete;

  task<ResultType> to_task();
  future(const task<ResultType> & task);
  future(task<ResultType> && task);

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



 } // namespace rpc
} // namespace dds


#endif // VENDOR_DEPENDENT_H
