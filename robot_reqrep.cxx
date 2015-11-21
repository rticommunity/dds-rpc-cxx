#include <cstdio>
#include <cstdlib>
#include <stdexcept>

#include "robotSupport.h"
#include "normative/request_reply.h"
#include "unique_data.h"

using namespace dds::rpc;
using namespace robot;

void test_synchronous_api(
    Requester<RobotControl_Request, RobotControl_Reply> & requester)
{
    helper::unique_data<RobotControl_Request> request;
    dds::Sample<RobotControl_Reply> reply_sample;
    float speed = 0, oldspeed = 0;

    for (int i = 0; i < 10; i++)
    {
	    request->data._d = RobotControl_getSpeed_Hash;
	    requester.send_request(*request);

      while (!requester.receive_reply(
                reply_sample,
                request->header.requestId,
                dds::Duration::from_seconds(20)))
      {
          printf("waiting for reply to getSpeed...\n");
      }

      oldspeed = reply_sample.data().data._u.getSpeed._u.result.return_;
      //printf("test_synchronous_api: sn = %d, get speed = %f\n",
		  //       reply_sample.data().header.relatedRequestId.sequence_number.low,
		  //       oldspeed);

	    speed = oldspeed + 10;

	    request->data._d = RobotControl_setSpeed_Hash;
	    request->data._u.setSpeed.speed = speed;

	    requester.send_request(*request);
      while (!requester.receive_reply(
                reply_sample,
                request->header.requestId,
                dds::Duration::from_seconds(20)))
      {
          // synchronous
          printf("test_synchronous_api: waiting for reply to setSpeed...\n");
      }
      if (reply_sample.data().data._u.setSpeed._d == robot::TooFast_Ex_Hash)
      {
          printf("test_synchronous_api: Going too fast.\n");
      }
      else
      {
          oldspeed = reply_sample.data().data._u.setSpeed._u.result.return_;
          assert(oldspeed == (speed - 10));
      }
    }
}

void test_synchronous_future(
    Requester<RobotControl_Request, RobotControl_Reply> & requester)
{
    helper::unique_data<RobotControl_Request> request;
    dds::rpc::future<dds::Sample<RobotControl_Reply>> reply_future;
    dds::Sample<RobotControl_Reply> reply_sample;
    float speed = 0, oldspeed = 0;

    
    for (int i = 0; i < 10; i++)
    {
        try {
            request->data._d = RobotControl_getSpeed_Hash;
            reply_future = requester.send_request_async(*request);

            reply_sample = reply_future.get(); // synchronous
            oldspeed = reply_sample.data().data._u.getSpeed._u.result.return_;

            //printf("test_synchronous_future: sn = %d, get speed = %f\n",
            //    reply_sample.data().header.relatedRequestId.sequence_number.low,
            //    oldspeed);

            speed = oldspeed + 10;

            request->data._d = RobotControl_setSpeed_Hash;
            request->data._u.setSpeed.speed = speed;

            reply_future = requester.send_request_async(*request);
            reply_sample = reply_future.get(); // synchronous

            if (reply_sample.data().data._u.setSpeed._d == robot::TooFast_Ex_Hash)
            {
                printf("test_synchronous_future: Going too fast!\n");
            }
            else
            {
                oldspeed = reply_sample.data().data._u.setSpeed._u.result.return_;
                assert(oldspeed == (speed - 10));
            }
        }
        catch (std::runtime_error & ex)
        {
            printf("test_synchronous_future: Runtime exception: %s\n", ex.what());
        }
    }
}

void test_asynchronous_getSpeed(
    Requester<RobotControl_Request, RobotControl_Reply> & requester)
{
    helper::unique_data<RobotControl_Request> request;
    auto speed_lambda = [](future<float> && maybe_speed)
    {
        try {
            float speed = maybe_speed.get();
            printf("test_asynchronous_getSpeed: getSpeed = %f\n", speed);
        }
        catch (std::runtime_error & ex) {
            printf("test_asynchronous_getSpeed: Runtime exception: %s\n", ex.what());
            //throw; 
        }
        catch (std::exception & ex) {
            printf("test_asynchronous_getSpeed: generic exception: %s\n", ex.what());
            //throw; 
        }
        catch (...) {
            printf("test_asynchronous_getSpeed: Unknown exception\n");
        }
    };

#ifdef USE_PPLTASKS

    for (int i = 0; i < 10; i++)
    {
        request->data._d = RobotControl_getSpeed_Hash;

        requester
            .send_request_async(*request)
            .then([i](future<dds::Sample<RobotControl_Reply>> && reply_fut)
            {
                dds::Sample<RobotControl_Reply> reply_sample = reply_fut.get();
                return reply_sample.data().data._u.getSpeed._u.result.return_;
            })
            .then(speed_lambda);
            // what happens if speed_lambda throws?
    }

#endif // USE_PPLTASKS
}

void test_asynchronous_race(
    Requester<RobotControl_Request, RobotControl_Reply> & requester)
{
    helper::unique_data<RobotControl_Request> request;

#ifdef USE_PPLTASKS
    for (int i = 0; i < 10 ; ++i)
    {
        request->data._d = robot::RobotControl_getSpeed_Hash;

        requester
            .send_request_async(*request)
            .then([requester] (future<dds::Sample<RobotControl_Reply>> && reply_fut) 
        {
            dds::Sample<RobotControl_Reply> reply_sample = reply_fut.get();
            float speed = reply_sample.data().data._u.getSpeed._u.result.return_;
            helper::unique_data<RobotControl_Request> request;
            request->data._d = robot::RobotControl_setSpeed_Hash;
            request->data._u.setSpeed.speed = speed + 10;
            return const_cast<Requester<RobotControl_Request, RobotControl_Reply> &>(requester).send_request_async(*request);
        })
        .then([](future<dds::Sample<RobotControl_Reply>> && reply_fut)
        {
            try {
                dds::Sample<RobotControl_Reply> reply_sample = reply_fut.get();
                
                if (reply_sample.data().data._d == robot::RobotControl_setSpeed_Hash)
                {
                    if(reply_sample.data().data._u.setSpeed._d == RETCODE_OK)
                    {
                        float oldspeed = reply_sample.data().data._u.setSpeed._u.result.return_;
                        printf("test_asynchronous_race: old speed = %f\n", oldspeed);
                    }
                    else if (reply_sample.data().data._u.setSpeed._d == TooFast_Ex_Hash)
                    {
                        printf("test_asynchronous_race: Going too fast!\n");
                    }
                    else
                    {
                        printf("test_asynchronous_race: Server-side exception\n");
                    }
                }
            }
            catch (std::runtime_error & ex) {
                printf("test_asynchronous_race: Run-time exception: %s\n", ex.what());
            }
            catch (std::exception & ex) {
                printf("test_asynchronous_race: Generic exception: %s\n", ex.what());
            }
            catch (...) {
                printf("test_asynchronous_race: Unknown exception\n");
            }
        });
    }
#endif // USE_PPLTASKS
}

#ifdef USE_AWAIT
#ifndef RTI_WIN32
future<void> test_await(
    Requester<RobotControl_Request, RobotControl_Reply> & requester)
{
    helper::unique_data<RobotControl_Request> request;



    request->data._d = robot::RobotControl_getSpeed_Hash;
    dds::Sample<RobotControl_Reply> reply = await requester.send_request_async(*request);
    printf("await current speed = %f\n", reply.data().data._u.getSpeed._u.result.return_);
}
#endif // RTI_WIN32
#endif // USE_AWAIT

void client_rr(int domainid, const std::string & service_name)
{
  dds::dds_entity_traits::DomainParticipant participant =
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

  NDDSUtility::sleep(dds::Duration::from_seconds(1));

  helper::unique_data<RobotControl_Request> request;
  dds::Sample<RobotControl_Reply> reply_sample;
  int i = 0;
  float speed = 0;

  test_synchronous_api(requester);
  test_synchronous_future(requester);
  test_asynchronous_getSpeed(requester);
  test_asynchronous_race(requester);

  printf("Press any key to end the program.\n");
  getchar();

#ifdef USE_AWAIT
#ifndef RTI_WIN32
  // test_await(requester).get();
#endif // RTI_WIN32
#endif // USE_AWAIT

}

void print_request(const RobotControl_Request & request)
{
  switch (request.data._d)
  {
    case RobotControl_command_Hash:
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
		}
		break;
	case RobotControl_setSpeed_Hash:
		printf("setSpeed = %f\n", request.data._u.setSpeed.speed);
		break;
	case RobotControl_getSpeed_Hash:
		printf("getSpeed\n");
		break;
	case RobotControl_getStatus_Hash:
		printf("getStatus\n");
		break;
  }
}

dds::SampleIdentity to_rpc_sample_identity(const DDS::SampleIdentity_t & inid)
{
	static_assert(sizeof(dds::SampleIdentity) == sizeof(DDS::SampleIdentity_t), 
		            "Sizes of two SampleIdentity don't match!");

	dds::SampleIdentity outid;
	memcpy(&outid, &inid, sizeof(SampleIdentity_t));
	return outid;
}

class Robot
{
	float speed;
  static const int SPEED_LIMIT = 100;

public:
	Robot() : speed(0) 
  { }

	helper::unique_data<RobotControl_Reply> process_request(
		const dds::Sample<RobotControl_Request> & request)
	{
    NDDSUtility::sleep(dds::Duration::from_millis(50));

		helper::unique_data<RobotControl_Reply> reply;
		float oldspeed = speed;

		switch (request.data().data._d)
		{
		    case RobotControl_command_Hash:
			    switch (request.data().data._u.command.com)
			    {
				    case robot::START_COMMAND:
				    case robot::STOP_COMMAND:
					    reply->data._d = RobotControl_command_Hash;
					    reply->data._u.command._d = RETCODE_OK;
					    reply->data._u.command._u.result.dummy = 0x0;
					    break;
				    default:
					    reply->data._d = RobotControl_command_Hash;
					    reply->data._u.command._d = RETCODE_ERROR;
					    reply->data._u.command._u.unknownEx = RETCODE_ERROR;
			    }
			    break;
		    case RobotControl_setSpeed_Hash:
            if (request.data().data._u.setSpeed.speed > SPEED_LIMIT)
            {
                reply->data._d = RobotControl_setSpeed_Hash;
                reply->data._u.setSpeed._d = robot::TooFast_Ex_Hash;
                reply->data._u.setSpeed._u.toofast_ex = TooFast();
                speed = 0;
            }
            else 
            {
                speed = request.data().data._u.setSpeed.speed;
                reply->data._d = RobotControl_setSpeed_Hash;
                reply->data._u.setSpeed._d = RETCODE_OK;
                reply->data._u.setSpeed._u.result.return_ = oldspeed;
            }
			    break;
		    case RobotControl_getSpeed_Hash:
			    reply->data._d = RobotControl_getSpeed_Hash;
			    reply->data._u.getSpeed._d = RETCODE_OK;
			    reply->data._u.getSpeed._u.result.return_ = speed;
			    break;
		    case RobotControl_getStatus_Hash:
			    reply->data._d = RobotControl_getStatus_Hash;
			    reply->data._u.getStatus._d = RETCODE_OK;
			    reply->data._u.getStatus._u.result.status.msg = _strdup("Good Status");
			    break;
		}

		return reply;
	}
};

void server_rr(int domainid, const std::string & service_name)
{
  dds::dds_entity_traits::DomainParticipant participant =
    TheParticipantFactory->create_participant(
    domainid,
    DDS::PARTICIPANT_QOS_DEFAULT,
    NULL /* listener */,
    DDS::STATUS_MASK_NONE);

  ReplierParams replier_params =
    dds::rpc::ReplierParams()
      .domain_participant(participant)
      .service_name(service_name);

  Robot robot;

  Replier<RobotControl_Request, RobotControl_Reply>
    replier(replier_params);

  while (true)
  {
    dds::Sample<RobotControl_Request> request;

    if (replier.receive_request(request,
                                dds::Duration::from_seconds(60)))
    {
      print_request(request.data());

      helper::unique_data<RobotControl_Reply> 
		  reply(robot.process_request(request));

      replier.send_reply(*reply, to_rpc_sample_identity(request.identity()));
    }
    else
      printf("timeout or invalid data. Ignoring...\n");
  }
}
