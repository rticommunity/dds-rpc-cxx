#ifndef ROBOTCONTROL_SUPPORT_H
#define ROBOTCONTROL_SUPPORT_H

#include "function_call.h"
#include "request_reply.h"
#include "unique_data.h"

#include "robotPlugin.h"
#include "robotSupport.h"

namespace robot {

  class RobotControlSupport;

  class RobotControl
  {
  public:

    typedef RobotControl         InterfaceType;
    typedef RobotControlSupport  SupportType;
    typedef RobotControl_Request RequestType;
    typedef RobotControl_Reply   ReplyType;

    virtual void command(const Command & command) = 0;
    virtual float setSpeed(float speed) = 0;
    virtual float getSpeed() = 0;
    virtual void getStatus(Status & status) = 0;

    virtual ~RobotControl() { }
  };

  class RobotControlAsync
  {
  public:

    typedef RobotControl         InterfaceType;
    typedef RobotControlSupport  SupportType;
    typedef RobotControl_Request RequestType;
    typedef RobotControl_Reply   ReplyType;

    virtual dds::rpc::future<void> command_async(const robot::Command & command) = 0;
    virtual dds::rpc::future<float> setSpeed_async(float speed) = 0;
    virtual dds::rpc::future<float> getSpeed_async() = 0;
    virtual dds::rpc::future<robot::RobotControl_getStatus_Out> getStatus_async() = 0;

    virtual ~RobotControlAsync() { }
  };

  class RobotControlSupport
  {
  public:
    typedef robot::RobotControl         InterfaceType;
    typedef robot::RobotControl_Request RequestType;
    typedef robot::RobotControl_Reply   ReplyType;

    class Client : public virtual robot::RobotControl,
                   public virtual robot::RobotControlAsync,
                   public virtual dds::rpc::ClientEndpoint
    {
    public:

      Client();

      Client(const dds::rpc::ClientParams & client_params);

      /* Methods from RobotControl */
      void command(const Command & command);
      float setSpeed(float speed);
      float getSpeed();
      void getStatus(Status & status);

      /* Methods from RobotControlAsync */
      dds::rpc::future<void> command_async(const robot::Command & command);
      dds::rpc::future<float> setSpeed_async(float speed);
      dds::rpc::future<float> getSpeed_async();
      dds::rpc::future<robot::RobotControl_getStatus_Out> getStatus_async();

    };

    class Service : public dds::rpc::ServiceEndpoint
    {
    public:

      template <class Impl>
      Service(Impl & impl);

      template <class Impl>
      Service(Impl & impl, dds::rpc::Server & server);

      template <class Impl>
      Service(Impl & impl,
        dds::rpc::Server & server,
        const dds::rpc::ServiceParams & service_params);

    };

  };

} // namespace robot

namespace dds {
  namespace rpc {
    namespace details {

      template <>
      class Dispatcher<robot::RobotControl>
        : public ServiceEndpointImpl
      {
      public:
        typedef robot::RobotControl                       InterfaceType;
        typedef robot::RobotControl::RequestType          RequestType;
        typedef robot::RobotControl::ReplyType            ReplyType;
        typedef dds::rpc::Replier<RequestType, ReplyType> Replier;

      private:
        robot::RobotControl * robotimpl_;
        ServiceParams service_params_;
        Replier replier_;
        
        void dispatch(const dds::Duration_t &);

      public:

        Dispatcher(robot::RobotControl & service_impl);
        Dispatcher(robot::RobotControl & service_impl, 
                   const ServiceParams & params);

        virtual void run(const dds::Duration_t &) override;

      };

      template <>
      class ClientImpl<robot::RobotControl>
        : public ClientEndpointImpl,
          public robot::RobotControl,
          public robot::RobotControlAsync
      {
      public:

        ClientImpl();

        ClientImpl(const dds::rpc::ClientParams & client_params);

        void bind(const std::string & instance_name) override;
        void unbind() override;
        bool is_bound() const override;
        std::string get_bound_instance_name() const override;
        std::vector<std::string> get_discovered_service_instances() const override;
        
        void wait_for_service() override;
        void wait_for_service(const dds::Duration_t & maxWait) override;

        void wait_for_service(std::string instanceName) override;
        void wait_for_service(const dds::Duration_t & maxWait,
                              std::string instanceName) override;

        void wait_for_services(int count) override;
        void wait_for_services(const dds::Duration_t & maxWait, int count) override;

        void wait_for_services(const std::vector<std::string> & instanceNames) override;
        void wait_for_services(const dds::Duration_t & maxWait,
                               const std::vector<std::string> & instanceNames) override;

        future<void> wait_for_service_async() override;
        future<void> wait_for_service_async(std::string instanceName) override;
        future<void> wait_for_services_async(int count) override;
        future<void> wait_for_services_async(const std::vector<std::string> & instanceNames) override;

        void close() override;

        dds::rpc::ClientParams get_client_params() const override;

        /* methods from RobotControl */
        void command(const robot::Command & command) override;
        float setSpeed(float speed) override;
        float getSpeed() override;
        void getStatus(robot::Status & status) override;

        /* methods from RobotControlAsync */
        dds::rpc::future<void> command_async(const robot::Command & command) override;
        dds::rpc::future<float> setSpeed_async(float speed) override;
        dds::rpc::future<float> getSpeed_async() override;
        dds::rpc::future<robot::RobotControl_getStatus_Out> getStatus_async() override;

      private:
        typedef dds::rpc::Requester<
          RobotControl::RequestType,
          RobotControl::ReplyType>
            Requester;

        dds::rpc::ClientParams params_;
        Requester requester_;
      };

    } // namespace details
  } // namespace rpc
} // namespace dds

#include "RobotControlSupport.hpp"

#endif // ROBOTCONTROL_SUPPORT_H
