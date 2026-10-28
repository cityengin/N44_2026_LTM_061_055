// PacketTag ghi thời điểm TẠO gói (ns). Tag không làm tăng số byte trên dây.
#pragma once

#include "ns3/tag.h"

#include <cstdint>

namespace noc
{

class CreationTimeTag : public ns3::Tag
{
  public:
    CreationTimeTag() = default;
    explicit CreationTimeTag(int64_t createdNs);

    static ns3::TypeId GetTypeId();
    ns3::TypeId GetInstanceTypeId() const override;
    uint32_t GetSerializedSize() const override;
    void Serialize(ns3::TagBuffer i) const override;
    void Deserialize(ns3::TagBuffer i) override;
    void Print(std::ostream& os) const override;

    int64_t GetCreatedNs() const { return m_createdNs; }

  private:
    int64_t m_createdNs = 0;
};

} // namespace noc
