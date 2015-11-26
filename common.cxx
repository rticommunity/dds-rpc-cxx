#include "common.h"
#include "rpc_types.h"
#include "ndds/ndds_requestreply_cpp.h"

bool operator < (
  const DDS_SampleIdentity_t & lhs,
  const DDS_SampleIdentity_t & rhs)
{
  return lhs.sequence_number < rhs.sequence_number;
}

namespace dds {

  bool operator < (
    const dds::SampleIdentity & lhs,
    const dds::SampleIdentity & rhs)
  {
    return lhs.sequence_number.low < rhs.sequence_number.low;
  }

  namespace rpc {

    namespace details {

      DefaultDomainParticipant::DefaultDomainParticipant()
        : domainid(0),
          participant(0)
      { }

      DefaultDomainParticipant & DefaultDomainParticipant::singleton()
      {
        static DefaultDomainParticipant default_participant;
        return default_participant;
      }

      DefaultDomainParticipant & DefaultDomainParticipant::set_domainid(int domainid)
      {
        this->domainid = domainid;
        return *this;
      }

      DDSDomainParticipant* DefaultDomainParticipant::get()
      {
        if(!participant)
          participant = TheParticipantFactory->create_participant(
                          domainid,
                          DDS::PARTICIPANT_QOS_DEFAULT,
                          NULL /* listener */,
                          DDS::STATUS_MASK_NONE);

        return participant;
      }
    }
  }
}
