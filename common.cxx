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
    return lhs.seqNum.low < rhs.seqNum.low;
  }

}
