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

  typedef ::DDS_Duration_t     Duration;
  typedef ::DDS_SampleInfo     SampleInfo;
  typedef ::DDS_SampleInfoSeq  SampleInfoSeq;  

  using connext::Sample;
  using connext::SampleRef;
  using connext::WriteSample;
  using connext::WriteSampleRef;
  using connext::LoanedSamples;
  using connext::SampleIterator;

  template <typename T>
  struct dds_type_traits 
  {
    typedef typename T::DataReader*  DataReader;
    typedef typename T::DataWriter*  DataWriter;

    typedef SampleRef<T>             SampleRefType;
    typedef SampleRef<T>             SampleIteratorValueType;
    typedef SampleRef<const T>       ConstSampleIteratorValueType;
    typedef LoanedSamples<T>         LoanedSamplesType;
    typedef SampleIterator<T, false> iterator;
    typedef SampleIterator<T, true>  const_iterator;
  };

  struct dds_entity_traits
  {
    typedef ::DDSDomainParticipant*       DomainParticipant;
    typedef ::DDSPublisher*               Publisher;
    typedef ::DDSSubscriber*              Subscriber;
    typedef ::DDS_DataReaderQos const *   DataReaderQos;
    typedef ::DDS_DataWriterQos const *   DataWriterQos;
  };

  template <class T>
  class SharedSamples;

  namespace rpc {

    class RPCEntity;
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
      class RPCEntityImpl;

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
      struct vendor_dependent<dds::rpc::RPCEntity>
      {
        typedef boost::shared_ptr<details::RPCEntityImpl> type;
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
#include "future_adapter.hpp"

#endif // VENDOR_DEPENDENT_H
