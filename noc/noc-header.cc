#include "noc-header.h"

namespace noc
{

NoCHeader::NoCHeader(uint16_t src, uint16_t dst, uint32_t seq)
    : m_src(src),
      m_dst(dst),
      m_seq(seq)
{
}

ns3::TypeId
NoCHeader::GetTypeId()
{
    static ns3::TypeId tid = ns3::TypeId("noc::NoCHeader")
                                 .SetParent<ns3::Header>()
                                 .SetGroupName("Noc")
                                 .AddConstructor<NoCHeader>();
    return tid;
}

ns3::TypeId
NoCHeader::GetInstanceTypeId() const
{
    return GetTypeId();
}

uint32_t
NoCHeader::GetSerializedSize() const
{
    return 2 + 2 + 4 + 2;
}

void
NoCHeader::Serialize(ns3::Buffer::Iterator start) const
{
    start.WriteHtonU16(m_src);
    start.WriteHtonU16(m_dst);
    start.WriteHtonU32(m_seq);
    start.WriteHtonU16(m_hops);
}

uint32_t
NoCHeader::Deserialize(ns3::Buffer::Iterator start)
{
    m_src = start.ReadNtohU16();
    m_dst = start.ReadNtohU16();
    m_seq = start.ReadNtohU32();
    m_hops = start.ReadNtohU16();
    return GetSerializedSize();
}

void
NoCHeader::Print(std::ostream& os) const
{
    os << "src=" << m_src << " dst=" << m_dst << " seq=" << m_seq << " hops=" << m_hops;
}

uint32_t
PayloadBytes()
{
    return kFrameBytes - kPppHeaderBytes - NoCHeader().GetSerializedSize();
}

} // namespace noc
