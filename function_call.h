#ifndef OMG_DDS_RPC_FUNCTION_CALL_H
#define OMG_DDS_RPC_FUNCTION_CALL_H

#include <string>
#include <vector>

#include "request_reply.h" // standard

#include "vendor_dependent.h"

namespace dds { namespace rpc { 

class Server; 
enum ServiceStatus { CLOSED, PAUSED, RUNNING };

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
public:
  ServiceParams();
  ServiceParams(const ServiceParams & other);
  ServiceParams & operator = (const ServiceParams & that);

  ServiceParams & service_name(const std::string &service_name);
  ServiceParams & instance_name(const std::string &instance_name);
  ServiceParams & request_topic_name(const std::string &req_topic);
  ServiceParams & reply_topic_name(const std::string &rep_topic);
  ServiceParams & datawriter_qos(const dds::DataWriterQos &qos);
  ServiceParams & datareader_qos(const dds::DataReaderQos &qos);
  ServiceParams & publisher(dds::Publisher *publisher);
  ServiceParams & subscriber(dds::Subscriber *subscriber);
  ServiceParams & domain_participant(dds::DomainParticipant *part);

  std::string service_name() const;
  std::string instance_name() const;
  std::string request_topic_name() const;
  std::string reply_topic_name() const;
  const dds::DataWriterQos * datawriter_qos() const;
  const dds::DataReaderQos * datareader_qos() const;
  dds::Publisher * publisher() const;
  dds::Subscriber * subscriber() const;
  dds::DomainParticipant * domain_participant() const;

protected:
  typedef details::vendor_dependent<ServiceParams>::type VendorDependent;
  VendorDependent impl_;

public:
  VendorDependent get_impl() const;
};

class ClientParams
{
public:
  ClientParams();
  ClientParams(const ClientParams & other);
  ClientParams & operator = (const ClientParams & that);

  ClientParams & service_name(const std::string &service_name);
  ClientParams & instance_name(const std::string &instance_name);
  ClientParams & request_topic_name(const std::string &req_topic);
  ClientParams & reply_topic_name(const std::string &rep_topic);
  ClientParams & datawriter_qos(const dds::DataWriterQos &qos);
  ClientParams & datareader_qos(const dds::DataReaderQos &qos);
  ClientParams & publisher(dds::Publisher *publisher);
  ClientParams & subscriber(dds::Subscriber *subscriber);
  ClientParams & domain_participant(dds::DomainParticipant *part);

  const std::string & service_name() const;
  const std::string & instance_name() const;
  const std::string & request_topic_name() const;
  const std::string & reply_topic_name() const;
  const dds::DataWriterQos * datawriter_qos() const;
  const dds::DataReaderQos * datareader_qos() const;
  dds::Publisher * publisher() const;
  dds::Subscriber * subscriber() const;
  dds::DomainParticipant * domain_participant() const;

protected:
  typedef details::vendor_dependent<ClientParams>::type VendorDependent;
  VendorDependent impl_;

public:
  VendorDependent get_impl() const;
};

class ServerParams
{
public:
  ServerParams();
  ServerParams(const ServerParams & other);
  ServerParams & operator = (const ServerParams & that);

  ServerParams & default_service_params(const ServiceParams & service_params);

  ServiceParams default_service_params() const;

protected:
  typedef details::vendor_dependent<ServerParams>::type VendorDependent;
  VendorDependent impl_;

public:
  VendorDependent get_impl() const;
};

class ServiceEndpoint
{
public:

  template <class Impl>
  explicit ServiceEndpoint(Impl impl);

  template <class TReq>
  typename dds_type_traits<TReq>::DataReader  get_request_datareader() const;

  template <class TRep>
  typename dds_type_traits<TRep>::DataWriter get_reply_datawriter() const;

  void pause();
  void resume();
  void close();
  ServiceStatus status() const;
  ServiceParams get_service_params() const;

protected:
  typedef details::vendor_dependent<ServiceEndpoint>::type VendorDependent;
  VendorDependent impl_;

public:
  VendorDependent get_impl() const;
};

class ClientEndpoint : public ServiceProxy
{
public:

  template <class Impl>
  explicit ClientEndpoint(Impl impl);

  template <class TReq>
  typename dds_type_traits<TReq>::DataWriter get_request_datawriter() const;

  template <class TRep>
  typename dds_type_traits<TRep>::DataReader get_reply_datareader() const;

  dds::rpc::ClientParams get_client_params();
};

class Server
{
public:
   // may reuse an exising participant
   Server();

   explicit Server(const ServerParams & server_params);

   // blocking
   void run();

   // non-blocking
   void run(const dds::Duration_t & max_wait);

protected:
  typedef details::vendor_dependent<Server>::type VendorDependent;
  VendorDependent impl_;

public:
   VendorDependent get_impl() const;

};

} // namespace rpc
} // namespace dds

#include "function_call.hpp"

#endif // OMG_DDS_RPC_FUNCTION_CALL_H

