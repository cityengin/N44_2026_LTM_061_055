// Header NoC trên dây (làm tăng số byte trên dây): src, dst, seq, hops = 10 B.
// Frame = 2 B PPP + NoCHeader + payload = 32 B.
#pragma once

#include "ns3/header.h"

#include <cstdint>

namespace noc
{

constexpr uint32_t kFrameBytes = 32;
constexpr uint32_t kPppHeaderBytes = 2;

class NoCHeader : public ns3::Header
{
  public:
    NoCHeader() = default;
    NoCHeader(uint16_t src, uint16_t dst, uint32_t seq);

    static ns3::TypeId GetTypeId();
    ns3::TypeId GetInstanceTypeId() const override;
    uint32_t GetSerializedSize() const override;
    void Serialize(ns3::Buffer::Iterator start) const override;
    uint32_t Deserialize(ns3::Buffer::Iterator start) override;
    void Print(std::ostream& os) const override;

    uint16_t GetSrc() const { return m_src; }
    uint16_t GetDst() const { return m_dst; }
    uint32_t GetSeq() const { return m_seq; }
    uint16_t GetHops() const { return m_hops; }
    void SetHops(uint16_t hops) { m_hops = hops; }

  private:
    uint16_t m_src = 0;
    uint16_t m_dst = 0;
    uint32_t m_seq = 0;
    uint16_t m_hops = 0; // số link đã đi qua, tăng khi nhận
};

// Payload = 32 − 2 (PPP) − kích thước NoCHeader.
uint32_t PayloadBytes();

} // namespace noc
