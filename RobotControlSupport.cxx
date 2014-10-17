#include "RobotControlSupport.h"

namespace robot {

  RobotControlSupport::Client::Client()
    : dds::rpc::ClientEndpoint(
        new dds::rpc::details::ClientImpl<robot::RobotControl>())
  {
      if (!impl_)
        throw std::runtime_error("Could not create robot::RobotControl::Client");
  }

  RobotControlSupport::Client::Client(
    const dds::rpc::ClientParams & client_params)
    : dds::rpc::ClientEndpoint(
        new dds::rpc::details::ClientImpl<robot::RobotControl>(client_params))
  {
    if (!impl_)
      throw std::runtime_error("Could not create robot::RobotControl::Client");
  }

  /***********************************************/
  /* Sync methods */
  /***********************************************/
  void RobotControlSupport::Client::command(const Command & command)
  {
    auto impl = static_cast<dds::rpc::details::ClientImpl<robot::RobotControl> *>(impl_.get());
    return impl->command(command);
  }

  float RobotControlSupport::Client::setSpeed(float speed)
  {
    auto impl = static_cast<dds::rpc::details::ClientImpl<robot::RobotControl> *>(impl_.get());
    return impl->setSpeed(speed);
  }

  float RobotControlSupport::Client::getSpeed()
  {
    auto impl = static_cast<dds::rpc::details::ClientImpl<robot::RobotControl> *>(impl_.get());
    return impl->getSpeed();
  }

  void RobotControlSupport::Client::getStatus(Status & status)
  {
    auto impl = static_cast<dds::rpc::details::ClientImpl<robot::RobotControl> *>(impl_.get());
    return impl->getStatus(status);
  }


  /***********************************************/
  /* Async methods */
  /***********************************************/
  dds::rpc::future<void> 
    RobotControlSupport::Client::command_async(const robot::Command & command)
  {
    auto impl = static_cast<dds::rpc::details::ClientImpl<robot::RobotControl> *>(impl_.get());
    return impl->command_async(command);
  }

  dds::rpc::future<float> 
    RobotControlSupport::Client::setSpeed_async(float speed)
  {
    auto impl = static_cast<dds::rpc::details::ClientImpl<robot::RobotControl> *>(impl_.get());
    return impl->setSpeed_async(speed);
  }

  dds::rpc::future<float> 
    RobotControlSupport::Client::getSpeed_async()
  {
    auto impl = static_cast<dds::rpc::details::ClientImpl<robot::RobotControl> *>(impl_.get());
    return impl->getSpeed_async();
  }

  dds::rpc::future<robot::RobotControl_getStatus_Out> 
    RobotControlSupport::Client::getStatus_async()
  {
    auto impl = static_cast<dds::rpc::details::ClientImpl<robot::RobotControl> *>(impl_.get());
    return impl->getStatus_async();
  }


} // namespace robot


namespace dds {
  namespace rpc {
    namespace details {

      rpc::ReplierParams
        to_replier_params(const rpc::ServiceParams & service_params)
      {
          return rpc::ReplierParams().domain_participant(service_params.domain_participant());
      }

      Dispatcher<robot::RobotControl>::Dispatcher(robot::RobotControl & service_impl)
        : robotimpl_(&service_impl),
          service_params_(), // FIXME: default construction
          replier_(to_replier_params(service_params_))
      { }

      Dispatcher<robot::RobotControl>::Dispatcher(
            robot::RobotControl & service_impl,
            const ServiceParams & service_params)
        : robotimpl_(&service_impl),
          service_params_(service_params),
          replier_(to_replier_params(service_params_))
      { }

      bool is_command(const Sample<robot::RobotControl_Request> & request_sample)
      {
        return request_sample.data().data._d == robot::RobotControl_command_hash;
      }

      bool is_getSpeed(const Sample<robot::RobotControl_Request> & request_sample)
      {
        return request_sample.data().data._d == robot::RobotControl_getSpeed_hash;
      }

      bool is_setSpeed(const Sample<robot::RobotControl_Request> & request_sample)
      {
        return request_sample.data().data._d == robot::RobotControl_setSpeed_hash;
      }

      bool is_getStatus(const Sample<robot::RobotControl_Request> & request_sample)
      {
        return request_sample.data().data._d == robot::RobotControl_getStatus_hash;
      }

      unique_data<robot::RobotControl_Reply> do_command(
        const Sample<robot::RobotControl_Request> & request_sample,
        robot::RobotControl * service_impl)
      {
        unique_data<robot::RobotControl_Reply> reply;

        service_impl->command(request_sample.data().data._u.command.com);

        reply->data._d = robot::RobotControl_command_hash;
        reply->data._u.command._d = dds::rpc::SUCCESS_RETCODE;
        reply->data._u.command._u.result.dummy = 0;

        return reply;
      }

      unique_data<robot::RobotControl_Reply> do_setSpeed(
        const Sample<robot::RobotControl_Request> & request_sample,
        robot::RobotControl * service_impl)
      {
        unique_data<robot::RobotControl_Reply> reply;

        try
        {
          float speed =
            service_impl->setSpeed(request_sample.data().data._u.setSpeed.speed);
          //printf("do_setSpeed = %d\n", request_sample.data().data._u.setSpeed.speed);

          reply->data._d = robot::RobotControl_setSpeed_hash;
          reply->data._u.setSpeed._d = dds::rpc::SUCCESS_RETCODE;
          reply->data._u.setSpeed._u.result.return_ = speed;
        }
        catch (robot::TooFast & toofast)
        {
          reply->data._d = robot::RobotControl_setSpeed_hash;
          reply->data._u.setSpeed._d = robot::RobotControl_getSpeed_TooFast_Ex_hash;
          reply->data._u.setSpeed._u.toofast_ex = toofast;
        }

        return reply;
      }

      unique_data<robot::RobotControl_Reply> do_getSpeed(
        const Sample<robot::RobotControl_Request> & request_sample,
        robot::RobotControl * service_impl)
      {
        unique_data<robot::RobotControl_Reply> reply;

        float speed = service_impl->getSpeed();

        reply->data._d = robot::RobotControl_getSpeed_hash;
        reply->data._u.getSpeed._d = dds::rpc::SUCCESS_RETCODE;
        reply->data._u.getSpeed._u.result.return_ = speed;

        return reply;
      }

      unique_data<robot::RobotControl_Reply> do_getStatus(
        const Sample<robot::RobotControl_Request> & request_sample,
        robot::RobotControl * service_impl)
      {
        unique_data<robot::RobotControl_Reply> reply;

        reply->data._d = robot::RobotControl_getStatus_hash;
        reply->data._u.getStatus._d = dds::rpc::SUCCESS_RETCODE;
        service_impl->getStatus(reply->data._u.getStatus._u.result.status);

        return reply;
      }

      void Dispatcher<robot::RobotControl>::dispatch(const DDS::Duration_t & timeout)
      {
        Sample<RequestType> request_sample;
        unique_data<ReplyType> reply;

        if (replier_.receive_request(request_sample, timeout))
        {
          if (is_command(request_sample))
            reply = do_command(request_sample, robotimpl_);
          else if (is_getSpeed(request_sample))
            reply = do_getSpeed(request_sample, robotimpl_);
          else if (is_setSpeed(request_sample))
            reply = do_setSpeed(request_sample, robotimpl_);
          else if (is_getStatus(request_sample))
            reply = do_getStatus(request_sample, robotimpl_);
          else
          {
            reply->sysx = dds::rpc::UNKNOWN_OPERATION;
            reply->data._d = 0; // default
          }

          replier_.send_reply_connext(*reply, request_sample);
        }
        else
          printf("timeout or invalid sampleinfo. Ignoring...\n");
      }

      void Dispatcher<robot::RobotControl>::run(const DDS::Duration_t & timeout)
      {
        /*
        if (!replier_.get())
        {
          ServiceParams sp =
            ServiceEndpointImplDispatcher::get_service_impl()->get_service_params();

          replier_ =
            boost::make_shared<Replier>(sp.domain_participant(),
            sp.service_name());
        }
        */
        dispatch(timeout);
      }

      /***************************************************************************/
      /* ClientImpl */
      /***************************************************************************/

      static dds::rpc::RequesterParams 
        to_requester_params(const ClientParams & client_params)
      {
          return dds::rpc::RequesterParams();
      }

      ClientImpl<robot::RobotControl>::ClientImpl() 
        : params_(dds::rpc::ClientParams()),
          requester_(to_requester_params(params_))
      {  }


      ClientImpl<robot::RobotControl>::ClientImpl(
        const dds::rpc::ClientParams & client_params)
        : params_(client_params),
          requester_(to_requester_params(params_))
      { }

      void ClientImpl<robot::RobotControl>::bind(const std::string & instance_name)
      {
        requester_.bind(instance_name);
      }

      void ClientImpl<robot::RobotControl>::unbind()
      {
        requester_.unbind();
      }

      bool ClientImpl<robot::RobotControl>::is_bound() const
      {
        return requester_.is_bound();
      }

      std::string ClientImpl<robot::RobotControl>::get_bound_instance_name() const
      {
        return requester_.get_bound_instance_name();
      }

      std::vector<std::string> 
        ClientImpl<robot::RobotControl>::get_discovered_service_instances() const
      {
          return std::vector<std::string>();
      }

      void ClientImpl<robot::RobotControl>::wait_for_service()
      { }

      void ClientImpl<robot::RobotControl>::wait_for_service(
        const dds::Duration_t & maxWait) 
      { }

      void ClientImpl<robot::RobotControl>::wait_for_service(
        std::string instanceName)
      { }

      void ClientImpl<robot::RobotControl>::wait_for_service(
        const dds::Duration_t & maxWait,
        std::string instanceName)
      { }

      void ClientImpl<robot::RobotControl>::wait_for_services(int count)
      { }
      
      void ClientImpl<robot::RobotControl>::wait_for_services(
        const dds::Duration_t & maxWait,
        int count)
      { }

      void ClientImpl<robot::RobotControl>::wait_for_services(
        const std::vector<std::string> & instanceNames)
      { }
      
      void ClientImpl<robot::RobotControl>::wait_for_services(
        const dds::Duration_t & maxWait,
        const std::vector<std::string> & instanceNames)
      { }

      future<void> 
        ClientImpl<robot::RobotControl>::wait_for_service_async()
      {    
        return future<void>();
      }

      future<void> 
        ClientImpl<robot::RobotControl>::wait_for_service_async(
        std::string instanceName)
      {
        return future<void>();
      }

      future<void> 
        ClientImpl<robot::RobotControl>::wait_for_services_async(
        int count)
      {
          return future<void>();
      }
      
      future<void> 
        ClientImpl<robot::RobotControl>::wait_for_services_async(
        const std::vector<std::string> & instanceNames)
      {
          return future<void>();
      }

      ClientParams ClientImpl<robot::RobotControl>::get_client_params() const
      {
        return params_;
      }

      void ClientImpl<robot::RobotControl>::close() 
      { }

      void ClientImpl<robot::RobotControl>::command(const robot::Command & command)
      {
        unique_data<robot::RobotControl_Request> request;
        Sample<robot::RobotControl_Reply> reply_sample;

        request->data._d = robot::RobotControl_command_hash;
        request->data._u.command.com = command;

        requester_.send_request(*request);
        requester_.receive_reply(reply_sample, request->header.requestId);
        printf("reply received successfully from command %d\n",
          reply_sample.data().header.relatedRequestId.seqnum.low);
      }

      float ClientImpl<robot::RobotControl>::setSpeed(float speed)
      {
        unique_data<robot::RobotControl_Request> request;
        Sample<robot::RobotControl_Reply> reply_sample;

        request->data._d = robot::RobotControl_setSpeed_hash;
        request->data._u.setSpeed.speed = speed;

        requester_.send_request(*request);
        requester_.receive_reply(reply_sample, request->header.requestId);

        if (reply_sample.data().data._d == robot::RobotControl_setSpeed_hash)
        {
          printf("reply received successfully from setSpeed %d\n",
            reply_sample.data().header.relatedRequestId.seqnum.low);

          switch (reply_sample.data().data._u.setSpeed._d)
          {
            case dds::rpc::SUCCESS_RETCODE:
            {
                return reply_sample.data().data._u.setSpeed._u.result.return_;
                break;
            }
            case robot::RobotControl_getSpeed_TooFast_Ex_hash:
            {
                throw reply_sample.data().data._u.setSpeed._u.toofast_ex;
                break;
            }
            default:
            {
                throw std::runtime_error("Received unknown exception from setSpeed.");
                break;
            }
          }
        }
        else
        {
          throw std::runtime_error("Received unknown response for setSpeed.\n");
        }
      }

      float ClientImpl<robot::RobotControl>::getSpeed()
      {
        unique_data<robot::RobotControl_Request> request;
        Sample<robot::RobotControl_Reply> reply_sample;

        request->data._d = robot::RobotControl_getSpeed_hash;
        request->data._u.getSpeed.dummy = 0;

        requester_.send_request(*request);
        requester_.receive_reply(reply_sample, request->header.requestId);

        if (reply_sample.data().data._d == robot::RobotControl_getSpeed_hash)
        {
          printf("reply received successfully from getSpeed %d\n",
            reply_sample.data().header.relatedRequestId.seqnum.low);

          switch (reply_sample.data().data._u.getSpeed._d)
          {
            case dds::rpc::SUCCESS_RETCODE:
            {
              return reply_sample.data().data._u.getSpeed._u.result.return_;
              break;
            }
            default:
            {
              throw std::runtime_error("Received unknown exception from getSpeed.");
              break;
            }
          }
        }
        else
        {
          throw std::runtime_error("Received unknown response for getSpeed.\n");
        }
      }

      void ClientImpl<robot::RobotControl>::getStatus(robot::Status & status)
      {
        unique_data<robot::RobotControl_Request> request;
        Sample<robot::RobotControl_Reply> reply_sample;

        request->data._d = robot::RobotControl_getStatus_hash;
        request->data._u.getStatus.dummy = 0;

        requester_.send_request(*request);
        requester_.receive_reply(reply_sample, request->header.requestId);

        if (reply_sample.data().data._d == robot::RobotControl_getStatus_hash)
        {
          printf("reply received successfully from getStatus %d\n",
            reply_sample.data().header.relatedRequestId.seqnum.low);

          switch (reply_sample.data().data._u.getSpeed._d)
          {
            case dds::rpc::SUCCESS_RETCODE:
            {
              robot::Status_copy(&status, &reply_sample.data().data._u.getStatus._u.result.status);
              break;
            }
            default:
            {
              throw std::runtime_error("Received unknown exception from getStatus.");
              break;
            }
          }
        }
        else
        {
          throw std::runtime_error("Received unknown response for getStatus.\n");
        }
      }

      dds::rpc::future<void> 
        ClientImpl<robot::RobotControl>::command_async(
          const robot::Command & command)
      {
          return dds::rpc::future<void>();
      }

      dds::rpc::future<float> 
        ClientImpl<robot::RobotControl>::setSpeed_async(float speed)
      {
          return dds::rpc::future<float>();
      }
      
      dds::rpc::future<float> 
        ClientImpl<robot::RobotControl>::getSpeed_async()
      {
          return dds::rpc::future<float>();
      }

      dds::rpc::future<robot::RobotControl_getStatus_Out> 
        ClientImpl<robot::RobotControl>::getStatus_async()
      {
          return dds::rpc::future<robot::RobotControl_getStatus_Out>();
      }

    } // namespace details
  } // namespace rpc
} // namespace dds

/*
Client::Client()
: impl_(boost::make_shared<details::ClientImpl>())
{}

Client::Client(DDS::DomainParticipant * part,
  DDS::Publisher * pub,
  DDS::Subscriber * sub)
  : impl_(boost::make_shared<details::ClientImpl>(part, pub, sub))
{}

*/