/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2013 Jens Mittag, Tristan Gaugel
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Jens Mittag <jens.mittag@gmail.com>
 *         Tristan Gaugel <tristan.gaugel@kit.edu>
 */

#include "stdma-net-device.h"
#include "stdma-mac.h"
#include "ns3/wifi-phy.h"
#include "ns3/wifi-channel.h"
#include "ns3/wifi-remote-station-manager.h"
#include "ns3/llc-snap-header.h"
#include "ns3/packet.h"
#include "ns3/uinteger.h"
#include "ns3/pointer.h"
#include "ns3/node.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/log.h"

namespace stdma {

NS_LOG_COMPONENT_DEFINE ("StdmaNetDevice");
NS_OBJECT_ENSURE_REGISTERED (StdmaNetDevice);

ns3::TypeId
StdmaNetDevice::GetTypeId (void)
{
  static ns3::TypeId tid = ns3::TypeId ("stdma::StdmaNetDevice")
    .SetParent<ns3::NetDevice> ()
    .AddConstructor<StdmaNetDevice> ()
    .AddAttribute ("Mtu", "The MAC-level Maximum Transmission Unit",
                   ns3::UintegerValue (MAX_MSDU_SIZE - ns3::LLC_SNAP_HEADER_LENGTH),
                   ns3::MakeUintegerAccessor (&StdmaNetDevice::SetMtu,
                                              &StdmaNetDevice::GetMtu),
                   ns3::MakeUintegerChecker<uint16_t> (1, MAX_MSDU_SIZE - ns3::LLC_SNAP_HEADER_LENGTH))
    .AddAttribute ("Channel", "The channel attached to this device",
                   ns3::PointerValue (),
                   ns3::MakePointerAccessor (&StdmaNetDevice::DoGetChannel),
                   ns3::MakePointerChecker<ns3::WifiChannel> ())
    .AddAttribute ("Phy", "The PHY layer attached to this device.",
                   ns3::PointerValue (),
                   ns3::MakePointerAccessor (&StdmaNetDevice::GetPhy,
                                             &StdmaNetDevice::SetPhy),
                   ns3::MakePointerChecker<ns3::WifiPhy> ())
    .AddAttribute ("Mac", "The MAC layer attached to this device.",
                   ns3::PointerValue (),
                   ns3::MakePointerAccessor (&StdmaNetDevice::GetMac,
                                             &StdmaNetDevice::SetMac),
                   ns3::MakePointerChecker<StdmaMac> ())
  ;

  return tid;
}

StdmaNetDevice::StdmaNetDevice ()
  : m_configComplete (false),
    m_mtu(MAX_MSDU_SIZE - ns3::LLC_SNAP_HEADER_LENGTH),
    m_linkUp(false),
    m_ifIndex(0)
{
  NS_LOG_FUNCTION_NOARGS ();
}
StdmaNetDevice::~StdmaNetDevice ()
{
  NS_LOG_FUNCTION_NOARGS ();
}

void
StdmaNetDevice::DoDispose (void)
{
  NS_LOG_FUNCTION_NOARGS ();
  m_node = 0;
  m_mac->Dispose ();
  m_phy->Dispose ();
  m_mac = 0;
  m_phy = 0;
  // chain up.
  NetDevice::DoDispose ();
}

void
StdmaNetDevice::DoInitialize (void)
{
  m_phy->Initialize ();
  m_mac->Initialize ();
  NetDevice::DoInitialize ();
}

void
StdmaNetDevice::CompleteConfig (void)
{
  if (m_mac == 0
      || m_phy == 0
      || m_node == 0
      || m_configComplete)
    {
      NS_LOG_WARN("Complete config called with some of the entities not set");
      return;
    }
  m_mac->SetWifiPhy (m_phy);
  m_mac->SetForwardUpCallback (MakeCallback (&StdmaNetDevice::ForwardUp, this));
  m_mac->SetLinkUpCallback (MakeCallback (&StdmaNetDevice::LinkUp, this));
  m_mac->SetLinkDownCallback (MakeCallback (&StdmaNetDevice::LinkDown, this));
  m_configComplete = true;
  NS_LOG_DEBUG("Complete config called and all entities were set");
}

void
StdmaNetDevice::SetMac (ns3::Ptr<StdmaMac> mac)
{
  m_mac = mac;
  CompleteConfig ();
}
void
StdmaNetDevice::SetPhy (ns3::Ptr<ns3::WifiPhy> phy)
{
  m_phy = phy;
  CompleteConfig ();
}
ns3::Ptr<StdmaMac>
StdmaNetDevice::GetMac (void) const
{
  return m_mac;
}
ns3::Ptr<ns3::WifiPhy>
StdmaNetDevice::GetPhy (void) const
{
  return m_phy;
}
void
StdmaNetDevice::SetIfIndex (const uint32_t index)
{
  m_ifIndex = index;
}
uint32_t
StdmaNetDevice::GetIfIndex (void) const
{
  return m_ifIndex;
}
ns3::Ptr<ns3::Channel>
StdmaNetDevice::GetChannel (void) const
{
  return m_phy->GetChannel ();
}
ns3::Ptr<ns3::WifiChannel>
StdmaNetDevice::DoGetChannel (void) const
{
  return m_phy->GetChannel ();
}
void
StdmaNetDevice::SetAddress (ns3::Address address)
{
  m_mac->SetAddress (ns3::Mac48Address::ConvertFrom (address));
}
ns3::Address
StdmaNetDevice::GetAddress (void) const
{
  return m_mac->GetAddress ();
}
bool
StdmaNetDevice::SetMtu (const uint16_t mtu)
{
  if (mtu > MAX_MSDU_SIZE - ns3::LLC_SNAP_HEADER_LENGTH)
    {
      return false;
    }
  m_mtu = mtu;
  return true;
}
uint16_t
StdmaNetDevice::GetMtu (void) const
{
  return m_mtu;
}
bool
StdmaNetDevice::IsLinkUp (void) const
{
  return m_phy != 0 && m_linkUp;
}
void
StdmaNetDevice::AddLinkChangeCallback (ns3::Callback<void> callback)
{
  m_linkChanges.ConnectWithoutContext (callback);
}
bool
StdmaNetDevice::IsBroadcast (void) const
{
  return true;
}
ns3::Address
StdmaNetDevice::GetBroadcast (void) const
{
  return ns3::Mac48Address::GetBroadcast ();
}
bool
StdmaNetDevice::IsMulticast (void) const
{
  return true;
}
ns3::Address
StdmaNetDevice::GetMulticast (ns3::Ipv4Address multicastGroup) const
{
  return ns3::Mac48Address::GetMulticast (multicastGroup);
}
ns3::Address
StdmaNetDevice::GetMulticast (ns3::Ipv6Address addr) const
{
  return ns3::Mac48Address::GetMulticast (addr);
}
bool
StdmaNetDevice::IsPointToPoint (void) const
{
  return false;
}
bool
StdmaNetDevice::IsBridge (void) const
{
  return false;
}
bool
StdmaNetDevice::Send (ns3::Ptr<ns3::Packet> packet, const ns3::Address& dest, uint16_t protocolNumber)
{
  NS_ASSERT (ns3::Mac48Address::IsMatchingType (dest));

  ns3::Mac48Address realTo = ns3::Mac48Address::ConvertFrom (dest);

  ns3::LlcSnapHeader llc;
  llc.SetType (protocolNumber);
  packet->AddHeader (llc);

  m_mac->Enqueue (packet, realTo);
  return true;
}
ns3::Ptr<ns3::Node>
StdmaNetDevice::GetNode (void) const
{
  return m_node;
}
void
StdmaNetDevice::SetNode (ns3::Ptr<ns3::Node> node)
{
  m_node = node;
  CompleteConfig ();
}
bool
StdmaNetDevice::NeedsArp (void) const
{
  return true;
}
void
StdmaNetDevice::SetReceiveCallback (NetDevice::ReceiveCallback cb)
{
  m_forwardUp = cb;
}
void
StdmaNetDevice::ForwardUp (ns3::Ptr<ns3::Packet> packet, ns3::Mac48Address from, ns3::Mac48Address to)
{
  ns3::LlcSnapHeader llc;
  packet->RemoveHeader (llc);
  enum NetDevice::PacketType type;
  if (to.IsBroadcast ())
    {
      type = NetDevice::PACKET_BROADCAST;
    }
  else if (to.IsGroup ())
    {
      type = NetDevice::PACKET_MULTICAST;
    }
  else if (to == m_mac->GetAddress ())
    {
      type = NetDevice::PACKET_HOST;
    }
  else
    {
      type = NetDevice::PACKET_OTHERHOST;
    }

  if (type != NetDevice::PACKET_OTHERHOST)
    {
      m_forwardUp (this, packet, llc.GetType (), from);
    }

  if (!m_promiscRx.IsNull ())
    {
      m_promiscRx (this, packet, llc.GetType (), from, to, type);
    }
}
void
StdmaNetDevice::LinkUp (void)
{
  m_linkUp = true;
  m_linkChanges ();
}
void
StdmaNetDevice::LinkDown (void)
{
  m_linkUp = false;
  m_linkChanges ();
}
bool
StdmaNetDevice::SendFrom (ns3::Ptr<ns3::Packet> packet, const ns3::Address& source, const ns3::Address& dest, uint16_t protocolNumber)
{
  NS_ASSERT (ns3::Mac48Address::IsMatchingType (dest));
  NS_ASSERT (ns3::Mac48Address::IsMatchingType (source));

  ns3::Mac48Address realTo = ns3::Mac48Address::ConvertFrom (dest);
  ns3::Mac48Address realFrom = ns3::Mac48Address::ConvertFrom (source);

  ns3::LlcSnapHeader llc;
  llc.SetType (protocolNumber);
  packet->AddHeader (llc);

  m_mac->Enqueue (packet, realTo, realFrom);

  return true;
}
void
StdmaNetDevice::SetPromiscReceiveCallback (PromiscReceiveCallback cb)
{
  m_promiscRx = cb;
}
bool
StdmaNetDevice::SupportsSendFrom (void) const
{
  return m_mac->SupportsSendFrom ();
}

} // namespace stdma
