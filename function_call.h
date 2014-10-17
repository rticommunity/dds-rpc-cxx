#ifndef OMG_DDS_RPC_FUNCTION_CALL_H
#define OMG_DDS_RPC_FUNCTION_CALL_H

#include <string>
#include <vector>

#include "request_reply.h"

#ifdef IMPLEMENTATION_DEPENDENT
#include "boost/shared_ptr.hpp"
#include "ndds/ndds_namespace_cpp.h"
#endif

namespace dds { namespace rpc { 

#ifdef IMPLEMENTATION_DEPENDENT
  namespace details 
  {
    class ServerImpl;

    template <class Iface>
    class ClientImpl;

    class ServiceParamsImpl;
    class ClientParamsImpl;
    class ServiceEndpointImpl;
  }

using DDS::Duration_t;
using DDS::DomainParticipant;
using DDS::DataReaderQos;
using DDS::DataWriterQos;

#endif // IMPLEMENTATION_DEPENDENT

class Server; 
enum ServiceStatus { PAUSED, RUNNING };

template <class Iface>
struct interface_traits
{
  typedef Iface                        InterfaceType;
  typedef typename Iface::ProxyType    ProxyType;
  typedef typename Iface::RequestType  RequestType;
  typedef typename Iface::ReplyType    ReplyType;
};

class Exception : public std::exception
{
  std::string message_;

public:
  Exception();
  Exception(const char *msg);

  virtual ~Exception() throw();
  virtual void raise() const = 0;
};

class ServiceParams
{
#ifdef IMPLEMENTATION_DEPENDENT
  boost::shared_ptr<details::ServiceParamsImpl> impl_;
#endif

public:
  ServiceParams();

  ServiceParams & service_name(const std::string &service_name);
  ServiceParams & instance_name(const std::string &instance_name);
  ServiceParams & request_topic_name(const std::string &req_topic);
  ServiceParams & reply_topic_name(const std::string &rep_topic);
  ServiceParams & datawriter_qos(const DDS::DataWriterQos &qos);
  ServiceParams & datareader_qos(const DDS::DataReaderQos &qos);
  ServiceParams & publisher(DDS::Publisher *publisher);
  ServiceParams & subscriber(DDS::Subscriber *subscriber);
  ServiceParams & domain_participant(DDS::DomainParticipant *part);

  std::string service_name() const;
  std::string instance_name() const;
  std::string request_topic_name() const;
  std::string reply_topic_name() const;
  const DDS::DataWriterQos * datawriter_qos() const;
  const DDS::DataReaderQos * datareader_qos() const;
  DDS::Publisher * publisher() const;
  DDS::Subscriber * subscriber() const;
  DDS::DomainParticipant * domain_participant() const;
};

class ClientParams
{
#ifdef IMPLEMENTATION_DEPENDENT
  boost::shared_ptr<details::ClientParamsImpl> impl_;
#endif

public:
  ClientParams();

  ClientParams & service_name(const std::string &service_name);
  ClientParams & instance_name(const std::string &instance_name);
  ClientParams & request_topic_name(const std::string &req_topic);
  ClientParams & reply_topic_name(const std::string &rep_topic);
  ClientParams & datawriter_qos(const DDS::DataWriterQos &qos);
  ClientParams & datareader_qos(const DDS::DataReaderQos &qos);
  ClientParams & publisher(DDS::Publisher *publisher);
  ClientParams & subscriber(DDS::Subscriber *subscriber);
  ClientParams & domain_participant(DDS::DomainParticipant *part);

  const std::string & service_name() const;
  const std::string & instance_name() const;
  const std::string & request_topic_name() const;
  const std::string & reply_topic_name() const;
  const DDS::DataWriterQos * datawriter_qos() const;
  const DDS::DataReaderQos * datareader_qos() const;
  DDS::Publisher * publisher() const;
  DDS::Subscriber * subscriber() const;
  DDS::DomainParticipant * domain_participant() const;
};

//typedef std::vector<ServiceProxy> ServiceProxyList;


class ServiceEndpoint
{
#ifdef IMPLEMENTATION_DEPENDENT
protected:
  boost::shared_ptr<details::ServiceEndpointImpl> impl_;
#endif

public:

  template <class Impl>
  ServiceEndpoint(Impl impl);

  template <class TReq>
  typename rpc_type_traits<TReq>::DataReader  get_request_datareader() const;

  template <class TRep>
  typename rpc_type_traits<TRep>::DataWriter get_reply_datawriter() const;

  void pause();

  void resume();

  void close();

  ServiceStatus status() const;

  ServiceParams get_service_params() const;

};

class ClientEndpoint : public ServiceProxy
{
public:

  template <class Impl>
  ClientEndpoint(Impl impl);

  template <class TReq>
  typename rpc_type_traits<TReq>::DataWriter get_request_datawriter() const;

  template <class TRep>
  typename rpc_type_traits<TRep>::DataReader get_reply_datareader() const;

  dds::rpc::ClientParams get_client_params();
};

class ServiceImplBase
{
protected:
  ServiceParams params;

public:
  virtual const ServiceParams & get_service_params() const;
  virtual void set_service_params(const ServiceParams &);
};
/*
class ServiceHandle
{
public:

  template <class ServiceImpl>
  ServiceHandle(ServiceImpl);

  void unregister();

  void pause();
  void resume();
  ServiceStatus status();

private:
  boost::shared_ptr<details::ServiceHandleImpl> impl_;
};
*/
class Server
{
public:
  // may reuse an exising participant
   Server();

   explicit Server(DDS::DomainParticipant * part,
                   DDS::Publisher * pub = 0,
                   DDS::Subscriber * sub = 0);

   // blocking
   void run();

   // non-blocking
   void run(const DDS::Duration_t & max_wait);
   
   typedef boost::shared_ptr<details::ServerImpl> unspecified;
   unspecified get_impl() const;

 private:
   boost::shared_ptr<details::ServerImpl> impl_;
};

} // namespace rpc
} // namespace dds

#include "function_call.hpp"

#endif // OMG_DDS_RPC_FUNCTION_CALL_H

/*
class Client
{
boost::shared_ptr<details::ClientImpl> impl_;

public:
// may reuse an exising participant
Client();

explicit Client(int domainid);

explicit Client(DDS::DomainParticipant * part,
DDS::Publisher * pub = 0,
DDS::Subscriber * sub = 0);

template <class Iface>
typename Iface::ProxyType
resolve_service(
const std::string & service_name);

template <class Iface>
typename Iface::ProxyType
resolve_service_instance(
const std::string & service_name,
const std::string & instance_name);

template <class Iface>
ServiceProxy
resolve_service_instances(
const std::string & service_name);

template <class Iface>
typename Iface::ProxyType
resolve_service(
const ServiceParams &);
};
*/