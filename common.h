#ifndef OMG_DDS_RPC_COMMON_H
#define OMG_DDS_RPC_COMMON_H

class DDSDomainParticipant;
struct DDS_SampleIdentity_t;

bool operator < (
  const DDS_SampleIdentity_t & lhs,
  const DDS_SampleIdentity_t & rhs);

namespace dds {

  class SampleIdentity;

  bool operator < (
    const dds::SampleIdentity & lhs,
    const dds::SampleIdentity & rhs);

  namespace rpc {

    namespace details {

      class DefaultDomainParticipant
      {
          int domainid;
          DDSDomainParticipant* participant;
          DefaultDomainParticipant();

        public:
          static DefaultDomainParticipant & singleton();
          DefaultDomainParticipant & set_domainid(int domainid);
          DDSDomainParticipant*  get();
      };

    } // namespace details
  } // namespace rpc
} // namespace dds

#endif // OMG_DDS_RPC_COMMON_H
