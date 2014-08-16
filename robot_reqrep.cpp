#include <stdio.h>
#include <stdlib.h>
#include <stdexcept>

#include "robotSupport.h"

#include "request_reply.h"
#include "unique_data.h"

using namespace dds::rpc;
using namespace robot;

void client_rr(int domainid, const std::string & service_name)
{
  DDS::DomainParticipant * participant =
    TheParticipantFactory->create_participant(
    domainid,
    DDS::PARTICIPANT_QOS_DEFAULT,
    NULL /* listener */,
    DDS::STATUS_MASK_NONE);

  Requester<RobotControl_Request, RobotControl_Reply>
    requester(participant, service_name);

  NDDSUtility::sleep(DDS::Duration_t::from_seconds(1));

  unique_data<RobotControl_Request> request;
  Sample<RobotControl_Reply> reply_sample;
  int i = 0;
  //while (true)
  {
    request->data._d = RobotControl_command_hash;
    request->data._u.command.com = robot::START_COMMAND;

    requester.send_request(*request);
    requester.receive_reply(reply_sample, request->header.requestId);
    printf("reply received successfully %d\n",
      i = reply_sample.data().header.relatedRequestId.seqnum.low);
  }

  //while (true)
  {
    future<Sample<RobotControl_Reply>> reply_fut =
      requester.send_request_async(*request);

    reply_sample = reply_fut.get();

    printf("reply received successfully %d\n",
      i = reply_sample.data().header.relatedRequestId.seqnum.low);
  }

#ifdef USE_PPLTASKS
  while (i < 10)
  {
    requester
      .send_request_async(*request)
      .then([&i](future<Sample<RobotControl_Reply>> && reply_fut)
    {
      try {
        Sample<RobotControl_Reply> reply_sample = reply_fut.get();

        printf("reply received successfully %d\n",
          i = reply_sample.data().header.relatedRequestId.seqnum.low);
      }
      catch (std::runtime_error & ex) {
        printf("Run-time exception from future: %s\n", ex.what());
      }
      catch (std::exception & ex)
      {
        printf("Generic exception from future: %s\n", ex.what());
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
  DDS::DomainParticipant * participant =
    TheParticipantFactory->create_participant(
    domainid,
    DDS::PARTICIPANT_QOS_DEFAULT,
    NULL /* listener */,
    DDS::STATUS_MASK_NONE);

  Replier<RobotControl_Request, RobotControl_Reply>
    replier(participant, service_name);

  while (true)
  {
    Sample<RobotControl_Request> request_sample;

    if (replier.receive_request(request_sample,
      DDS::Duration_t::from_seconds(60)))
    {
      print_request(request_sample.data());

      unique_data<RobotControl_Reply> reply;
      reply->data._d = RobotControl_command_hash;
      reply->data._u.command._d = dds::rpc::SUCCESS_RETCODE;
      //reply->data._u.command._u.result.dummy = 0x0;

      replier.send_reply_ex(*reply, request_sample);
    }
    else
      printf("timeout or invalid sampleinfo. Ignoring...\n");
  }
}
