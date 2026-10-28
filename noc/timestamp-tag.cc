#include "timestamp-tag.h"

namespace noc
{

CreationTimeTag::CreationTimeTag(int64_t createdNs)
    : m_createdNs(createdNs)
{
}

ns3::TypeId
CreationTimeTag::GetTypeId()
{
    static ns3::TypeId tid = ns3::TypeId("noc::CreationTimeTag")
                                 .SetParent<ns3::Tag>()
                                 .SetGroupName("Noc")
                                 .AddConstructor<CreationTimeTag>();
    return tid;
}

ns3::TypeId
CreationTimeTag::GetInstanceTypeId() const
{
    return GetTypeId();
}

uint32_t
CreationTimeTag::GetSerializedSize() const
{
    return 8;
}

void
CreationTimeTag::Serialize(ns3::TagBuffer i) const
{
    i.WriteU64(static_cast<uint64_t>(m_createdNs));
}

void
CreationTimeTag::Deserialize(ns3::TagBuffer i)
{
    m_createdNs = static_cast<int64_t>(i.ReadU64());
}

void
CreationTimeTag::Print(std::ostream& os) const
{
    os << "created=" << m_createdNs << "ns";
}

} // namespace noc
