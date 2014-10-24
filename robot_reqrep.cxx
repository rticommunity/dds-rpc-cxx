#include <stdio.h>
#include <stdlib.h>
#include <stdexcept>

#include "robotSupport.h"

#include "request_reply.h"
#include "unique_data.h"

using namespace dds;
using namespace dds::rpc;
using namespace robot;

void client_rr(int domainid, const std::string & service_name)
{
  dds::DomainParticipant * participant =
    TheParticipantFactory->create_participant(
    domainid,
    DDS::PARTICIPANT_QOS_DEFAULT,
    NULL /* listener */,
    DDS::STATUS_MASK_NONE);

  RequesterParams requester_params =
    dds::rpc::RequesterParams()
      .domain_participant(participant)
      .service_name(service_name);

  Requester<RobotControl_Request, RobotControl_Reply>
    requester(requester_params);

  NDDSUtility::sleep(dds::Duration_t::from_seconds(1));

  unique_data<RobotControl_Request> request;
  Sample<RobotControl_Reply> reply_sample;
  int i = 0;

  //while (true)
  {
    request->data._d = RobotControl_setSpeed_hash;
    request->data._u.setSpeed.speed = 55;

    requester.send_request(*request);
    requester.receive_reply(reply_sample, request->dds_rpc_request_header.requestId);
    printf("reply received successfully %d\n",
      i = reply_sample.data().dds_rpc_reply_header.relatedRequestId.seqnum.low);
  }

  //while (true)
  {
    future<Sample<RobotControl_Reply>> reply_fut =
      requester.send_request_async(*request);

    reply_sample = reply_fut.get();

    printf("reply received successfully %d\n",
      i = reply_sample.data().dds_rpc_reply_header.relatedRequestId.seqnum.low);
  }

  auto int_lambda = [](future<int> && fint)
  {
    try {
      int i = fint.get();
      printf("finished int %d\n", i);
    }
    catch (...) {
      printf("exception: int did not finish\n");
    }
  };

#ifdef USE_PPLTASKS
  while (i < 10)
  {
    requester
      .send_request_async(*request)
      .then([&](future<Sample<RobotControl_Reply>> && reply_fut)
        {
          try {
            Sample<RobotControl_Reply> reply_sample = reply_fut.get();

            printf("reply received successfully %d\n",
              i = reply_sample.data().dds_rpc_reply_header.relatedRequestId.seqnum.low);

            return 888;
          }
          catch (std::runtime_error & ex) {
            printf("Run-time exception from future: %s\n", ex.what());
            throw;
          }
          catch (std::exception & ex)
          {
            printf("Generic exception from future: %s\n", ex.what());
            throw;
          }
    })
      .then(int_lambda);

    printf("Waiting for a char %d:\n", i + 1);
    getchar();
  }
#endif // USE_PPLTASKS

#ifdef USE_PPLTASKS
  while (i < 20)
  {
    requester
      .send_request_async(*request)
      .then([&](future<Sample<RobotControl_Reply>> && reply_fut)
          {
            try {
              Sample<RobotControl_Reply> reply_sample = reply_fut.get();

              printf("reply received successfully %d\n",
                i = reply_sample.data().dds_rpc_reply_header.relatedRequestId.seqnum.low);

              request->data._d = RobotControl_getStatus_hash;
              return requester.send_request_async(*request);
            }
            catch (std::runtime_error & ex) {
              printf("Run-time exception from future: %s\n", ex.what());
              throw;
            }
            catch (std::exception & ex)
            {
              printf("Generic exception from future: %s\n", ex.what());
              throw;
            }
          })
       .then([](future<Sample<RobotControl_Reply>> && reply_fut)
         {
           try {
             Sample<RobotControl_Reply> reply_sample = reply_fut.get();

             if (reply_sample.data().data._u.getStatus._d == dds::rpc::SUCCESS_RETCODE)
             {
               printf("second .then reply received successfully %s\n",
                      reply_sample.data().data._u.getStatus._u.result.status.msg);
             }
           }
           catch (std::runtime_error & ex) {
              printf("second .then Run-time exception from future: %s\n", ex.what());
           }
           catch (std::exception & ex)
           {
              printf("second .then Generic exception from future: %s\n", ex.what());
           }
         });

    printf("Waiting for a char %d:\n", i + 1);
    getchar();
  }
#endif // USE_PPLTASKS
}

void print_request(const RobotControl_Request & request)
{
  if (request.data._d == RobotControl_command_hash)
  {
    switch (request.data._u.command.com)
    {
    case robot::START_COMMAND:
      printf("START_COMMAND\n");
      break;
    case robot::STOP_COMMAND:
      printf("START_COMMAND\n");
      break;
    default:
      printf("Unknown\n");
    };
  }
}

void server_rr(int domainid, const std::string & service_name)
{
  dds::DomainParticipant * participant =
    TheParticipantFactory->create_participant(
    domainid,
    DDS::PARTICIPANT_QOS_DEFAULT,
    NULL /* listener */,
    DDS::STATUS_MASK_NONE);

  ReplierParams replier_params =
    dds::rpc::ReplierParams()
      .domain_participant(participant)
      .service_name(service_name);

  Replier<RobotControl_Request, RobotControl_Reply>
    replier(replier_params);

  while (true)
  {
    Sample<RobotControl_Request> request_sample;

    if (replier.receive_request(request_sample,
                                dds::Duration_t::from_seconds(60)))
    {
      print_request(request_sample.data());

      unique_data<RobotControl_Reply> reply;
      reply->data._d = RobotControl_command_hash;
      reply->data._u.command._d = dds::rpc::SUCCESS_RETCODE;
      //reply->data._u.command._u.result.dummy = 0x0;

      // FIXME replier.send_reply_connext(*reply, request_sample);
    }
    else
      printf("timeout or invalid sampleinfo. Ignoring...\n");
  }
}
