#include "unique_data.h"
#include "request_reply.h"

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

        void dispatch(const dds::Duration &);

      public:

        Dispatcher(robot::RobotControl & service_impl);
        Dispatcher(robot::RobotControl & service_impl,
          const ServiceParams & params);

        virtual void run(const dds::Duration &) override;

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
        void wait_for_service(const dds::Duration & maxWait) override;

        void wait_for_service(std::string instanceName) override;
        void wait_for_service(const dds::Duration & maxWait,
          std::string instanceName) override;

        void wait_for_services(int count) override;
        void wait_for_services(const dds::Duration & maxWait, int count) override;

        void wait_for_services(const std::vector<std::string> & instanceNames) override;
        void wait_for_services(const dds::Duration & maxWait,
          const std::vector<std::string> & instanceNames) override;

        future<void> wait_for_service_async() override;
        future<void> wait_for_service_async(std::string instanceName) override;
        future<void> wait_for_services_async(int count) override;
        future<void> wait_for_services_async(const std::vector<std::string> & instanceNames) override;

        void close() override;

        /* Methods from ClientEndpointImpl */
        DDS::DataWriter * get_request_datawriter() const;
        DDS::DataReader * get_reply_datareader() const;
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

namespace robot {

  template <class Impl>
  RobotControlSupport::Service::Service(Impl & impl)
    : ServiceEndpoint(new dds::rpc::details::Dispatcher<RobotControl>(impl))
  { 
    // server.get_impl()->register_service(impl_);
  }

  template <class Impl>
  RobotControlSupport::Service::Service(
        Impl & impl, 
        dds::rpc::Server & server)
    : ServiceEndpoint(new dds::rpc::details::Dispatcher<RobotControl>(impl)) // ignored = server
  { 
    server.get_impl()->register_service(impl_);
  }

  template <class Impl>
  RobotControlSupport::Service::Service(
          Impl & impl,
          dds::rpc::Server & server,
          const dds::rpc::ServiceParams & service_params)
    : ServiceEndpoint(new dds::rpc::details::Dispatcher<RobotControl>(impl, service_params)) // ignored = server, service_params
  { 
    server.get_impl()->register_service(impl_);
  }

} // namespace robot

