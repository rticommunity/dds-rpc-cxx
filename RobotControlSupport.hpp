
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

