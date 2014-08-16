#ifndef OMG_DDS_RPC_FUNCTION_CALL_H
#define OMG_DDS_RPC_FUNCTION_CALL_H

#include <string>
#include <vector>

#ifdef IMPLEMENTATION_DEPENDENT
#include "boost/shared_ptr.hpp"
#include "ndds/ndds_namespace_cpp.h"
#endif

namespace dds { namespace rpc { 

#ifdef IMPLEMENTATION_DEPENDENT
  namespace details 
  {
    class ServerImpl;
    class ServiceHandleImpl;
    class ClientImpl;
    class ServiceProxyImplBase;

    template <class Iface>
    class ServiceProxyImpl;
  }

using DDS::DomainParticipant;
using DDS::DataReaderQos;
using DDS::DataWriterQos;

#endif // IMPLEMENTATION_DEPENDENT

class Executor;
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
  DDS::DomainParticipant * participant_;
  DDS::Publisher * publisher_;
  DDS::Subscriber * subscriber_;
  DDS::DataWriterQos dwqos_; bool dwqos_def;
  DDS::DataReaderQos drqos_; bool drqos_def;

  Server * server_;

  std::string service_name_;
  std::string instance_name_;
  std::string request_topic_name_;
  std::string reply_topic_name_;
#endif

public:
  ServiceParams();

  ServiceParams & server(Server &s);
  ServiceParams & service_name(const std::string &service_name);
  ServiceParams & instance_name(const std::string &instance_name);
  ServiceParams & request_topic_name(const std::string &req_topic);
  ServiceParams & reply_topic_name(const std::string &rep_topic);
  ServiceParams & datawriter_qos(const DDS::DataWriterQos &qos);
  ServiceParams & datareader_qos(const DDS::DataReaderQos &qos);
  ServiceParams & publisher(DDS::Publisher *publisher);
  ServiceParams & subscriber(DDS::Subscriber *subscriber);
  ServiceParams & domain_participant(DDS::DomainParticipant *part);

  const std::string & service_name() const;
  const std::string & instance_name() const;
  const std::string & request_topic_name() const;
  const std::string & reply_topic_name() const;
  const DDS::DataWriterQos * datawriter_qos() const;
  const DDS::DataReaderQos * datareader_qos() const;
  DDS::Publisher * publisher() const;
  DDS::Subscriber * subscriber() const;
  DDS::DomainParticipant * domain_participant() const;
  Server * server() const;
};

class ServiceProxy
{
#ifdef IMPLEMENTATION_DEPENDENT
  boost::shared_ptr<details::ServiceProxyImplBase> impl_;
#endif

public:
  
  template <class Impl>
  ServiceProxy(Impl impl);

  void bind(const std::string & instance_name);
  void unbind();
  bool is_bound() const;
  const std::string & bound_instance() const;
  const ServiceParams & service_params() const;
  details::ServiceProxyImplBase * get_impl() const;
};

typedef std::vector<ServiceProxy> ServiceProxyList;

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


class ServiceImplBase
{
protected:
  ServiceParams params;

public:
  virtual const ServiceParams & get_service_params() const;
  virtual void set_service_params(const ServiceParams &);
};

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

class Server
{
public:
  // may reuse an exising participant
   Server();

   Server(int domainid);

   explicit Server(DDS::DomainParticipant * part,
                   DDS::Publisher * pub = 0,
                   DDS::Subscriber * sub = 0);

   template <class ServiceImpl>
   ServiceHandle register_service(
     ServiceImpl &service_impl,
     const std::string service_name);

   // blocking
   void run();

   //blocking 
   void run(Executor &);
   
   // not blocking
   void run(const DDS::Duration_t & max_wait);
   
   Executor * executor();

   typedef boost::shared_ptr<details::ServerImpl> unspecified;
   unspecified get_impl();

 private:
   boost::shared_ptr<details::ServerImpl> impl_;
};

} // namespace rpc
} // namespace dds

#include "function_call.hpp"

#endif // OMG_DDS_RPC_FUNCTION_CALL_H

