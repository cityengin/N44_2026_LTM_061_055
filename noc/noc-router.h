// Router của một node mesh. Mọi quyết định định tuyến gọi routeSet (cùng hàm với bộ tính φ),
// sau đó SelectUniform chọn một hướng. Gói được tạo tại router nguồn bằng Inject.
#pragma once

#include "noc-header.h"
#include "routing.h"
#include "topology.h"

#include "ns3/address.h"
#include "ns3/net-device.h"
#include "ns3/packet.h"
#include "ns3/ptr.h"
#include "ns3/random-variable-stream.h"

#include <array>
#include <cstdint>
#include <functional>

namespace noc
{

class NocRouter
{
  public:
    // Gọi khi một gói tới đích: header (hops đã tính cả link cuối) và thời điểm tạo (ns).
    using DeliverFn = std::function<void(const NoCHeader& hdr, int64_t createdNs)>;

    NocRouter(int id, const Mesh& mesh, Algo algo, DeliverFn deliver);

    // dev: device trên node này nối tới hàng xóm theo hướng d.
    // Phải gọi SAU PointToPointHelper::Install, vì Node::AddDevice ghi đè receive callback.
    void AttachDevice(Dir d, ns3::Ptr<ns3::NetDevice> dev);

    // Gán stream cố định cho biến ngẫu nhiên chọn hướng. Trả về số stream đã dùng.
    int64_t AssignStreams(int64_t stream);

    // Tạo gói mới gửi tới dst (≠ id), gắn thời điểm tạo = Simulator::Now().
    void Inject(int dst, uint32_t seq);

  private:
    bool Receive(ns3::Ptr<ns3::NetDevice> dev,
                 ns3::Ptr<const ns3::Packet> packet,
                 uint16_t protocol,
                 const ns3::Address& from);
    void Forward(ns3::Ptr<ns3::Packet> packet, const NoCHeader& hdr);

    int m_id;
    Mesh m_mesh;
    Algo m_algo;
    DeliverFn m_deliver;
    std::array<ns3::Ptr<ns3::NetDevice>, kNumLinkDirs> m_devs;
    ns3::Ptr<ns3::UniformRandomVariable> m_selectRv;
};

} // namespace noc
