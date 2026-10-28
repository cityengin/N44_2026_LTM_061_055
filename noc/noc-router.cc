#include "noc-router.h"

#include "selection.h"
#include "timestamp-tag.h"

#include "ns3/abort.h"
#include "ns3/simulator.h"

namespace noc
{

NocRouter::NocRouter(int id, const Mesh& mesh, Algo algo, DeliverFn deliver)
    : m_id(id),
      m_mesh(mesh),
      m_algo(algo),
      m_deliver(std::move(deliver)),
      m_selectRv(ns3::CreateObject<ns3::UniformRandomVariable>())
{
}

void
NocRouter::AttachDevice(Dir d, ns3::Ptr<ns3::NetDevice> dev)
{
    m_devs[static_cast<int>(d)] = dev;
    dev->SetReceiveCallback(ns3::MakeCallback(&NocRouter::Receive, this));
}

int64_t
NocRouter::AssignStreams(int64_t stream)
{
    m_selectRv->SetStream(stream);
    return 1;
}

void
NocRouter::Inject(int dst, uint32_t seq)
{
    ns3::Ptr<ns3::Packet> p = ns3::Create<ns3::Packet>(PayloadBytes());
    const NoCHeader hdr(static_cast<uint16_t>(m_id), static_cast<uint16_t>(dst), seq);
    p->AddHeader(hdr);
    p->AddPacketTag(CreationTimeTag(ns3::Simulator::Now().GetNanoSeconds()));
    Forward(p, hdr);
}

bool
NocRouter::Receive(ns3::Ptr<ns3::NetDevice> /* dev */,
                   ns3::Ptr<const ns3::Packet> packet,
                   uint16_t /* protocol */,
                   const ns3::Address& /* from */)
{
    ns3::Ptr<ns3::Packet> p = packet->Copy();
    NoCHeader hdr;
    p->RemoveHeader(hdr);
    hdr.SetHops(hdr.GetHops() + 1);
    if (hdr.GetDst() == m_id)
    {
        CreationTimeTag tag;
        NS_ABORT_MSG_UNLESS(p->PeekPacketTag(tag), "gói thiếu CreationTimeTag");
        m_deliver(hdr, tag.GetCreatedNs());
        return true;
    }
    p->AddHeader(hdr);
    Forward(p, hdr);
    return true;
}

void
NocRouter::Forward(ns3::Ptr<ns3::Packet> packet, const NoCHeader& hdr)
{
    const DirSet s = routeSet(m_algo,
                              m_mesh.ToCoord(m_id),
                              m_mesh.ToCoord(hdr.GetSrc()),
                              m_mesh.ToCoord(hdr.GetDst()));
    const Dir d = SelectUniform(s, m_selectRv);
    ns3::Ptr<ns3::NetDevice> dev = m_devs[static_cast<int>(d)];
    NS_ABORT_MSG_UNLESS(dev, "node " << m_id << " không có link theo hướng " << DirName(d));
    // Send trả false khi queue tràn; drop được đếm qua trace "Drop" của queue.
    dev->Send(packet, dev->GetBroadcast(), 0x0800);
}

} // namespace noc
