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

#include "ns3/boolean.h"
#include "ns3/pointer.h"
#include "ns3/uinteger.h"
#include "ns3/double.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/simulator.h"
#include "ns3/log.h"
#include "ns3/nstime.h"
#include "ns3/llc-snap-header.h"
#include "ns3/yans-wifi-phy.h"
#include "ns3/mobility-model.h"
#include "ns3/string.h"
#include "ns3/enum.h"
#include "ns3/node.h"
#include "ns3/node-list.h"
#include "ns3/wifi-mac-trailer.h"
#include "ns3/wifi-tx-vector.h"

#include "stdma-mac.h"
#include "stdma-net-device.h"
#include "stdma-header.h"
#include <sstream>
#include <iostream>

namespace stdma
{

  NS_LOG_COMPONENT_DEFINE("StdmaMac");
  NS_OBJECT_ENSURE_REGISTERED(StdmaMac);

  StdmaMacPhyListener::StdmaMacPhyListener(StdmaMac *mac)
    : m_StdmaMac(mac)
  {
  }

  StdmaMacPhyListener::~StdmaMacPhyListener()
  {
  }

  void
  StdmaMacPhyListener::NotifyRxStart(ns3::Time duration)
  {
    m_StdmaMac->HandleRxStart (duration);
  }

  void
  StdmaMacPhyListener::NotifyRxEndOk(void)
  {
    m_StdmaMac->HandleRxSuccess();
  }

  void
  StdmaMacPhyListener::NotifyRxEndError(void)
  {
    m_StdmaMac->HandleRxFailure();
  }

  void
  StdmaMacPhyListener::NotifyTxStart(ns3::Time duration)
  {
  }

  void
  StdmaMacPhyListener::NotifyMaybeCcaBusyStart(ns3::Time duration)
  {
    m_StdmaMac->HandleMaybeCcaBusyStartNow(duration);
  }

  void
  StdmaMacPhyListener::NotifySwitchingStart(ns3::Time duration)
  {
  }

  ns3::TypeId
  StdmaMac::GetTypeId(void)
  {
    static ns3::TypeId tid = ns3::TypeId("stdma::StdmaMac")
        .SetParent<Object>().AddConstructor<StdmaMac>()
        .AddAttribute("FrameDuration",
                      "Define how long a superframe lasts. Tested value is the default (1s)",
                      ns3::TimeValue(ns3::Seconds(1)),
                      ns3::MakeTimeAccessor(&StdmaMac::m_frameDuration),
                      ns3::MakeTimeChecker())
        .AddAttribute("MaximumPacketSize",
                      "The maximum number of bytes per slot (excludes MAC STDMA and lower layer wrapping)."
                      "If packet is greater than this, it will be dropped ",
                      ns3::UintegerValue(500),
                      ns3::MakeUintegerAccessor(&StdmaMac::m_maxPacketSize),
                      ns3::MakeUintegerChecker<uint32_t>())
        .AddAttribute("Timeout",
                      "A RandomVariable used to determine the timeout of a reservation."
                      "According to the standard, one should use a UniformVariable(3,7) as a configuration here.",
                      ns3::RandomVariableValue (ns3::UniformVariable(3, 7)),
                      ns3::MakeRandomVariableAccessor (&StdmaMac::m_timeoutRng),
                      ns3::MakeRandomVariableChecker ())
        .AddAttribute("ReportRate",
                      "The desired number of transmissions per frame",
                      ns3::UintegerValue(2),
                      ns3::MakeUintegerAccessor(&StdmaMac::m_reportRate),
                      ns3::MakeUintegerChecker<uint8_t>())
        .AddAttribute("WifiMode",
                      "The WiFi mode to use for transmission",
                      ns3::WifiModeValue(ns3::WifiMode("OfdmRate6Mbps")),
                      ns3::MakeWifiModeAccessor(&StdmaMac::m_wifiMode),
                      ns3::MakeWifiModeChecker())
        .AddAttribute("WifiPreamble",
                      "The WiFi preamble mode",
                      ns3::EnumValue(ns3::WIFI_PREAMBLE_LONG),
                      ns3::MakeEnumAccessor(&StdmaMac::m_wifiPreamble),
                      ns3::MakeEnumChecker(ns3::WIFI_PREAMBLE_LONG, "Long WiFi preamble",
                                           ns3::WIFI_PREAMBLE_SHORT, "Short WiFi preamble"))
        .AddAttribute("GuardInterval",
                      "The guard interval added at the end of the slot in us (no transmission time)",
                      ns3::TimeValue(ns3::MicroSeconds(6)),
                      ns3::MakeTimeAccessor(&StdmaMac::m_guardInterval),
                      ns3::MakeTimeChecker())
        .AddAttribute("NumberOfRandomAccessSlots",
                      "The number of slots that shall be considered for random access in the network entry phase",
                      ns3::UintegerValue(150),
                      ns3::MakeUintegerAccessor(&StdmaMac::m_slotsForRtdma),
                      ns3::MakeUintegerChecker<uint16_t>())
        .AddAttribute("SelectionIntervalSize",
                      "The ratio that defines the size of the selection interval area, from which to choose a free slot randomly",
                      ns3::DoubleValue(0.2),
                      ns3::MakeDoubleAccessor(&StdmaMac::SetSelectionIntervalRatio,
                                              &StdmaMac::GetSelectionIntervalRatio),
                      ns3::MakeDoubleChecker<double>(0.0, 1.0))
        .AddAttribute("MinimumCandidateSetSize",
                      "Defines the minimum number of candidate slots required before randomly choosing one of them.",
                      ns3::UintegerValue(4),
                      ns3::MakeUintegerAccessor(&StdmaMac::SetMinimumCandidateSetSize),
                      ns3::MakeUintegerChecker<uint32_t>())
        .AddAttribute ("SlotManager",
                      "A reference to the slot manager object",
                      ns3::PointerValue (),
                      ns3::MakePointerAccessor (&StdmaMac::m_manager),
                      ns3::MakePointerChecker<StdmaSlotManager> ())
        .AddTraceSource("Startup",
                      "This event is triggered when the station starts up, i.e. when the initialization phase starts",
                      ns3::MakeTraceSourceAccessor (&StdmaMac::m_startupTrace))
        .AddTraceSource("NetworkEntry",
                      "This event is triggered when the station performs its network entry, i.e. it is transmitting its first packet",
                      ns3::MakeTraceSourceAccessor (&StdmaMac::m_networkEntryTrace))
        .AddTraceSource("Tx",
                      "This event is triggered whenever the station transmits a packet",
                      ns3::MakeTraceSourceAccessor (&StdmaMac::m_txTrace))
        .AddTraceSource("Rx",
                      "This event is triggered whenever the station receives a packet",
                      ns3::MakeTraceSourceAccessor (&StdmaMac::m_rxTrace));
    return tid;
  }

  StdmaMac::StdmaMac ()
   : m_phyListener(0),
     m_rxOngoing(false),
     m_rxStart(ns3::Seconds(0)),
     m_startedUp(false),
     m_manager(0)
  {
    // Queue to hold packets in
    m_queue = ns3::CreateObject<ns3::WifiMacQueue>();
    m_manager = ns3::CreateObject<StdmaSlotManager>();
  }

  StdmaMac::~StdmaMac ()
  {
    if (m_phyListener != 0)
      {
        delete (m_phyListener);
      }
  }

  void
  StdmaMac::ForwardUp (ns3::Ptr<ns3::Packet> packet, ns3::Mac48Address from, ns3::Mac48Address to)
  {
    NS_LOG_FUNCTION (this << packet << from);
    m_forwardUp (packet, from, to);
  }

  void
  StdmaMac::SetAddress (ns3::Mac48Address address)
  {
    m_self = address;
    SetBssid(address);
  }

  bool
  StdmaMac::SupportsSendFrom(void) const
  {
    return false;
  }

  void
  StdmaMac::SetWifiPhy(ns3::Ptr<ns3::WifiPhy> phy)
  {
    NS_LOG_FUNCTION (this << phy);
    m_phy = phy;
    // Stuff to do when events occur
    m_phy->SetReceiveOkCallback(ns3::MakeCallback(&StdmaMac::Receive, this));
    if (m_phyListener != 0)
      {
        delete (m_phyListener);
      }
    m_phyListener = new StdmaMacPhyListener (this);
  }

  void
  StdmaMac::Enqueue(ns3::Ptr<const ns3::Packet> packet, ns3::Mac48Address to, ns3::Mac48Address from)
  {
    NS_FATAL_ERROR("This MAC entity (" << this << ", " << GetAddress () << ") does not support Enqueue() with from address");
  }

  void
  StdmaMac::Enqueue(ns3::Ptr<const ns3::Packet> packet, ns3::Mac48Address to)
  {
    NS_LOG_FUNCTION(this << packet << to);
    ns3::WifiMacHeader hdr;
    hdr.SetTypeData();
    hdr.SetAddr1(to);
    hdr.SetAddr2(GetAddress());
    hdr.SetAddr3(GetBssid());
    hdr.SetDsNotFrom();
    hdr.SetDsNotTo();
    StdmaHeader stdmaHdr;
    ns3::WifiMacTrailer fcs;
    uint32_t numBytes = packet->GetSize() + stdmaHdr.GetSerializedSize() + hdr.GetSize() + fcs.GetSerializedSize();
    if (numBytes <= m_maxPacketSize)
      {
        NS_LOG_DEBUG(ns3::Simulator::Now() << " " << ns3::Simulator::GetContext() << " StdmaMac:Enqueue() packet with no. " << packet->GetUid() << " has been enqueued at node "
            << ns3::Simulator::GetContext() << " (size = " << numBytes << " bytes)");
        m_queue->Enqueue(packet, hdr);
      }
    else
      {
        NS_LOG_DEBUG(ns3::Simulator::Now() << " " << ns3::Simulator::GetContext() << " StdmaMac:Enqueue() packet with no. " << packet->GetUid() << " has been dropped ("
            << numBytes << " > MAX_PACKET_SIZE = " << m_maxPacketSize);
      }
  }

  void
  StdmaMac::StartInitializationPhase ()
  {
    NS_LOG_FUNCTION_NOARGS();

    // Create a STDMA slot manager that keeps track of what is going on on the wireless channel
    m_manager->Setup(ns3::Simulator::Now(), m_frameDuration, GetSlotDuration(), m_minimumCandidateSetSize);
    m_manager->SetReportRate(m_reportRate);
    m_manager->SetSelectionIntervalRatio(m_selectionIntervalRatio);

    // Get starting time of the first super frame
    ns3::Time start = m_manager->GetStart();
    uint64_t Ni = floor(1.0 * m_manager->GetSlotsPerFrame() / m_reportRate);
    uint64_t slotDuration = m_manager->GetFrameDuration().GetNanoSeconds() / m_manager->GetSlotsPerFrame();
    ns3::Time end = start + m_manager->GetFrameDuration() + ns3::NanoSeconds(Ni * slotDuration);

    // Schedule an event for the end of the initialization phase
    m_endInitializationPhaseEvent = ns3::Simulator::Schedule(end - ns3::Simulator::Now(), &StdmaMac::EndOfInitializationPhase, this);

    // Mark the MAC layer as started up (i.e. powered on)
    m_startedUp = true;
    m_startupTrace(start, m_manager->GetFrameDuration(), GetSlotDuration());
  }

  void
  StdmaMac::EndOfInitializationPhase ()
  {
    NS_LOG_FUNCTION(ns3::Simulator::GetContext());

    // Determine random access details of the network entry transmission
    ns3::Ptr<RandomAccessDetails> details = m_manager->GetNetworkEntryTimestamp(m_slotsForRtdma, 0.0);

    // Schedule an event for this transmission
    ns3::Time delay = details->GetWhen() - ns3::Simulator::Now();
    m_nextTransmissionEvent = ns3::Simulator::Schedule(delay, &StdmaMac::PerformNetworkEntry, this,
        details->GetRemainingSlots(), details->GetProbability());
    NS_LOG_DEBUG(ns3::Simulator::Now() << " " << ns3::Simulator::GetContext() << " StdmaMac:EndOfInitializationPhase() scheduled network entry " << delay.GetSeconds() << " in the future");
    NS_LOG_DEBUG(ns3::Simulator::Now() << " " << ns3::Simulator::GetContext() << " StdmaMac:EndOfInitializationPhase() refers to global slot id " << m_manager->GetGlobalSlotIndexForTimestamp(details->GetWhen()));
  }

  void
  StdmaMac::PerformNetworkEntry (uint32_t remainingSlots, double p)
  {
    NS_LOG_FUNCTION (this << ns3::Simulator::GetContext() << remainingSlots << p);
    NS_LOG_DEBUG(ns3::Simulator::Now() << " " << ns3::Simulator::GetContext() << " StdmaMac:PerformNetworkEntry() current global slot id: " << m_manager->GetGlobalSlotIndexForTimestamp(ns3::Simulator::Now()));
    NS_LOG_DEBUG(ns3::Simulator::Now() << " " << ns3::Simulator::GetContext() << " StdmaMac:PerformNetworkEntry() current slot = " << m_manager->GetSlotIndexForTimestamp(ns3::Simulator::Now()));

    // Sanity check: do we have properly assigned slot start times?
    ns3::Time baseTime = ns3::Simulator::Now() - m_manager->GetStart();
    uint64_t slotTime = m_manager->GetFrameDuration().GetNanoSeconds() / m_manager->GetSlotsPerFrame();
    NS_ASSERT (baseTime.GetNanoSeconds() % slotTime == 0);
    bool isTaken = false;

    // 0) Check if the current slot is still marked as free, if not: schedule a new
    //    event for PerformNetworkEntry following the RA-TDMA access rules
    if (!m_manager->IsCurrentSlotStillFree())
      {
        isTaken = true;
        NS_LOG_DEBUG(ns3::Simulator::Now() << " " << ns3::Simulator::GetContext() << " StdmaMac:PerformNetworkEntry() slot is not marked free anymore, consider a re-scheduling");

        if (remainingSlots > 0 && m_manager->HasFreeSlotsLeft(remainingSlots))
          {
            ns3::Ptr<RandomAccessDetails> details = m_manager->GetNetworkEntryTimestamp(remainingSlots, p);
            ns3::Time delay = details->GetWhen() - ns3::Simulator::Now();
            m_nextTransmissionEvent = ns3::Simulator::Schedule(delay, &StdmaMac::PerformNetworkEntry, this, details->GetRemainingSlots(), details->GetProbability());
            NS_LOG_DEBUG(ns3::Simulator::Now() << " " << ns3::Simulator::GetContext() << " StdmaMac:PerformNetworkEntry() there were still free slots afterward, scheduling a new network entry "
                << delay.GetSeconds() << " in the future");
            NS_LOG_DEBUG(ns3::Simulator::Now() << " " << ns3::Simulator::GetContext() << " StdmaMac:PerformNetworkEntry() refers to global slot id " << m_manager->GetGlobalSlotIndexForTimestamp(details->GetWhen()));
            return;
          }
      }

    // 1) Re-base the frame such that it starts at the next frame
    m_manager->RebaseFrameStart(ns3::Simulator::Now() + ns3::NanoSeconds(slotTime));

    // 2) Select the nominal start slot and the following nominal slots
    m_manager->SelectNominalSlots();

    // 3a) Select the first nominal transmission slot
    uint8_t timeout = floor(m_timeoutRng.GetValue() + 0.5);
    m_manager->SelectTransmissionSlotForReservationWithNo(0, timeout);
    ns3::Time delay = m_manager->GetTimeUntilTransmissionOfReservationWithNo(0);
    NS_ASSERT(delay.GetNanoSeconds() % slotTime == 0);
    uint32_t offset = (delay.GetNanoSeconds() / slotTime);
    NS_LOG_DEBUG(ns3::Simulator::Now() << " " << ns3::Simulator::GetContext() << " StdmaMac:PerformNetworkEntry() offset refers to global slot with id "
    		<< m_manager->GetGlobalSlotIndexForTimestamp(ns3::Simulator::Now() + ns3::NanoSeconds(offset * slotTime)));

    // 3b) Transmit the network entry packet
    ns3::WifiMacHeader wifiMacHdr;
    ns3::Ptr<ns3::Packet> packet = m_queue->Dequeue(&wifiMacHdr)->Copy();

    //     Create a STDMA header object and fill it properly...
    StdmaHeader stdmaHdr;
    stdmaHdr.SetOffset(offset);
    stdmaHdr.SetTimeout(0);
    ns3::Ptr<ns3::Node> myself = ns3::NodeList::GetNode(ns3::Simulator::GetContext());
    ns3::Ptr<ns3::MobilityModel> mobility = myself->GetObject<ns3::MobilityModel>();
    stdmaHdr.SetLatitude(mobility->GetPosition().x);
    stdmaHdr.SetLongitude(mobility->GetPosition().y);
    stdmaHdr.SetNetworkEntry();

    //     Create a frame check sequence trailer
    ns3::WifiMacTrailer fcs;
    const uint32_t slotBytes = packet->GetSize() + stdmaHdr.GetSerializedSize() + wifiMacHdr.GetSize() + fcs.GetSerializedSize();
    ns3::WifiTxVector txVector(m_wifiMode, 1, 0, false, 1, 1, false);
    ns3::Time txDuration = m_phy->CalculateTxDuration(slotBytes, txVector, m_wifiPreamble);
    wifiMacHdr.SetDuration(txDuration);

    //     Add everything to the packet
    packet->AddHeader(stdmaHdr);
    packet->AddHeader(wifiMacHdr);
    packet->AddTrailer(fcs);

    NS_LOG_DEBUG(ns3::Simulator::Now() << " " << ns3::Simulator::GetContext() << " StdmaMac:PerformNetworkEntry() transmitting packet " << packet->GetUid() << " with packet->GetSize() = " << packet->GetSize());
    NS_LOG_DEBUG(ns3::Simulator::Now() << " " << ns3::Simulator::GetContext() << " StdmaMac:PerformNetworkEntry() --> STDMA.latitude: " << stdmaHdr.GetLatitude());
    NS_LOG_DEBUG(ns3::Simulator::Now() << " " << ns3::Simulator::GetContext() << " StdmaMac:PerformNetworkEntry() --> STDMA.longitude: " << stdmaHdr.GetLongitude());
    NS_LOG_DEBUG(ns3::Simulator::Now() << " " << ns3::Simulator::GetContext() << " StdmaMac:PerformNetworkEntry() --> STDMA.timeout: " << (uint32_t) stdmaHdr.GetTimeout());
    NS_LOG_DEBUG(ns3::Simulator::Now() << " " << ns3::Simulator::GetContext() << " StdmaMac:PerformNetworkEntry() --> STDMA.offset: " << stdmaHdr.GetOffset());

    //     Send packet through physical layer
    NS_ASSERT_MSG (!m_phy->IsStateTx(), "StdmaMac:PerformNetworkEntry() physical layer should not be transmitting already.");
    m_phy->SendPacket(packet, m_wifiMode, m_wifiPreamble, txVector);
    NS_ASSERT (m_phy->IsStateTx ());
    m_networkEntryTrace(packet, delay, isTaken);

    // 4) Schedule an event for the next transmission
    m_nextTransmissionEvent = ns3::Simulator::Schedule(delay, &StdmaMac::DoTransmit, this, true);
  }

  void
  StdmaMac::DoTransmit(bool firstFrame)
  {
    NS_LOG_FUNCTION(this << firstFrame);
    NS_LOG_DEBUG(ns3::Simulator::Now() << " " << ns3::Simulator::GetContext() << " StdmaMac:DoTransmit() current global slot id: " << m_manager->GetGlobalSlotIndexForTimestamp(ns3::Simulator::Now()));

    // Sanity check: do we have properly assigned slot start times?
    ns3::Time baseTime = ns3::Simulator::Now() - m_manager->GetStart();
    uint64_t slotTime = m_manager->GetFrameDuration().GetNanoSeconds() / m_manager->GetSlotsPerFrame();
    NS_ASSERT (baseTime.GetNanoSeconds() % slotTime == 0);

    if (m_queue->IsEmpty())
      {
        NS_FATAL_ERROR("StdmaMac:DoTransmit() is called but no packets in queue which could be transmitted.");
      }

    // 1) Define the packet numbers (note: these are not the slot identifiers) of the current and the next transmission
    uint32_t current = m_manager->GetCurrentReservationNo();
    uint32_t next = ((current + 1) < m_reportRate) ? current + 1 : 0;
    NS_LOG_DEBUG(ns3::Simulator::Now() << " " << ns3::Simulator::GetContext() << " StdmaMac:DoTransmit() node " << ns3::Simulator::GetContext() << " is transmitting (current = "
                << current << ", next = " << next << "). t = " << ns3::Simulator::Now().GetSeconds());
    NS_LOG_DEBUG(ns3::Simulator::Now() << " " << ns3::Simulator::GetContext() << " StdmaMac:DoTransmit() current slot = " << m_manager->GetSlotIndexForTimestamp(ns3::Simulator::Now()));

    // 2a) Select a transmission slot for the next transmission if this is the first time this (next) packet is
    //    transmitted.
    if (firstFrame && (next > current))
      {
        uint8_t timeout = floor(m_timeoutRng.GetValue() + 0.5);
        m_manager->SelectTransmissionSlotForReservationWithNo(next, timeout);
      }

    // 2b) Schedule an event for the next packet transmission
    ns3::Time delay = m_manager->GetTimeUntilTransmissionOfReservationWithNo(next);
    NS_LOG_DEBUG(ns3::Simulator::Now() << " " << ns3::Simulator::GetContext() << " StdmaMac:DoTransmit() delay until next transmission event = " << delay.GetSeconds());
    bool stillFirstFrame = firstFrame && (current < next);
    NS_LOG_DEBUG(ns3::Simulator::Now() << " " << ns3::Simulator::GetContext() << " StdmaMac:DoTransmit() stillFirstFrame = " << stillFirstFrame);
    m_nextTransmissionEvent = ns3::Simulator::Schedule(delay, &StdmaMac::DoTransmit, this, stillFirstFrame);

    // 2c) Check whether the current packet number needs re-reservation. If yes, perform the re-reservation
    //     and calculate the offset for the next frame (which is the difference between the current slot index
    //     for this packet number, and the slot index to be used in the future).
    uint32_t offset;
    uint8_t timeout = m_manager->DecreaseTimeOutOfReservationWithNumber(current);
    if (m_manager->NeedsReReservation(current))
      {
        uint8_t timeout = floor(m_timeoutRng.GetValue() + 0.5);
        offset = m_manager->ReSelectTransmissionSlotForReservationWithNo(current, timeout);
        NS_LOG_DEBUG(ns3::Simulator::Now() << " " << ns3::Simulator::GetContext() << " StdmaMac:DoTransmit() offset for same reservation in next frame = " << offset);
        NS_LOG_DEBUG(ns3::Simulator::Now() << " " << ns3::Simulator::GetContext() << " StdmaMac:DoTransmit() offset refers to global slot with id " << m_manager->GetGlobalSlotIndexForTimestamp(ns3::Simulator::Now()) + offset);
      }
    else
      {
        offset = m_manager->CalculateSlotOffsetBetweenTransmissions(current, next);
        NS_LOG_DEBUG(ns3::Simulator::Now() << " " << ns3::Simulator::GetContext() << " StdmaMac:DoTransmit() offset to next transmission slot = " << offset);
        NS_LOG_DEBUG(ns3::Simulator::Now() << " " << ns3::Simulator::GetContext() << " StdmaMac:DoTransmit() offset refers to global slot with id " << m_manager->GetGlobalSlotIndexForTimestamp(ns3::Simulator::Now()) + offset);
      }


    // 3) Perform transmission, which means we need to get a packet from the transmission queue,
    //    add a STDMA header, and then forward it to the physical layer
    ns3::WifiMacHeader wifiMacHdr;
    ns3::Ptr<ns3::Packet> packet = m_queue->Dequeue(&wifiMacHdr)->Copy();

    // 3a) Create a STDMA header object and fill it properly...
    StdmaHeader stdmaHdr;
    stdmaHdr.SetOffset(offset);
    stdmaHdr.SetTimeout(timeout);
    ns3::Ptr<ns3::Node> myself = ns3::NodeList::GetNode(ns3::Simulator::GetContext());
    ns3::Ptr<ns3::MobilityModel> mobility = myself->GetObject<ns3::MobilityModel>();
    stdmaHdr.SetLatitude(mobility->GetPosition().x);
    stdmaHdr.SetLongitude(mobility->GetPosition().y);

    // 3b) Create a frame check sequence trailer
    ns3::WifiMacTrailer fcs;
    const uint32_t slotBytes = packet->GetSize() + stdmaHdr.GetSerializedSize() + wifiMacHdr.GetSize() + fcs.GetSerializedSize();
    ns3::WifiTxVector txVector(m_wifiMode, 1, 0, false, 1, 1, false);
    ns3::Time txDuration = m_phy->CalculateTxDuration(slotBytes, txVector, m_wifiPreamble);
    wifiMacHdr.SetDuration(txDuration);

    // 3c) Add everything to the packet
    packet->AddHeader(stdmaHdr);
    packet->AddHeader(wifiMacHdr);
    packet->AddTrailer(fcs);

    NS_LOG_DEBUG(ns3::Simulator::Now() << " " << ns3::Simulator::GetContext() << " StdmaMac:DoTransmit() transmitting packet " << packet->GetUid() << " with packet->GetSize() = " << packet->GetSize());
    NS_LOG_DEBUG(ns3::Simulator::Now() << " " << ns3::Simulator::GetContext() << " StdmaMac:DoTransmit() --> STDMA.latitude: " << stdmaHdr.GetLatitude());
    NS_LOG_DEBUG(ns3::Simulator::Now() << " " << ns3::Simulator::GetContext() << " StdmaMac:DoTransmit() --> STDMA.longitude: " << stdmaHdr.GetLongitude());
    NS_LOG_DEBUG(ns3::Simulator::Now() << " " << ns3::Simulator::GetContext() << " StdmaMac:DoTransmit() --> STDMA.timeout: " << (uint32_t) stdmaHdr.GetTimeout());
    NS_LOG_DEBUG(ns3::Simulator::Now() << " " << ns3::Simulator::GetContext() << " StdmaMac:DoTransmit() --> STDMA.offset: " << stdmaHdr.GetOffset());
    NS_LOG_DEBUG(ns3::Simulator::Now() << " " << ns3::Simulator::GetContext() << " StdmaMac:DoTransmit() current global slot id " << m_manager->GetGlobalSlotIndexForTimestamp(ns3::Simulator::Now()));

    // 3d) Send packet through physical layer
    NS_ASSERT_MSG (!m_phy->IsStateTx(), "StdmaMac:DoTransmit() physical layer should not be transmitting already.");
    m_phy->SendPacket(packet, m_wifiMode, m_wifiPreamble, txVector);
    NS_ASSERT (m_phy->IsStateTx ());
    m_txTrace(packet, current, timeout, offset);
  }

  ns3::Mac48Address
  StdmaMac::GetBssid(void) const
  {
    return m_bssid;
  }

  ns3::Mac48Address
  StdmaMac::GetAddress(void) const
  {
    return m_self;
  }

  void
  StdmaMac::SetBssid(ns3::Mac48Address bssid)
  {
    m_bssid = bssid;
  }

  ns3::Ssid
  StdmaMac::GetSsid(void) const
  {
    return m_ssid;
  }

  void
  StdmaMac::SetSsid(ns3::Ssid ssid)
  {
    m_ssid = ssid;
  }

  ns3::Time
  StdmaMac::GetGuardInterval(void) const
  {
    return m_guardInterval;
  }

  void
  StdmaMac::SetGuardInterval(const ns3::Time gi)
  {
    m_guardInterval = gi;
  }

  void
  StdmaMac::SetSelectionIntervalRatio(double ratio)
  {
    m_selectionIntervalRatio = ratio;
  }

  double
  StdmaMac::GetSelectionIntervalRatio() const
  {
    return m_selectionIntervalRatio;
  }

  void
  StdmaMac::SetMinimumCandidateSetSize (uint32_t size)
  {
    m_minimumCandidateSetSize = size;
    if (m_manager != 0)
      {
        m_manager->SetMinimumCandidateSlotSetSize(m_minimumCandidateSetSize);
      }
  }

  void
  StdmaMac::ConfigureStandard(enum ns3::WifiPhyStandard standard)
  {
    switch (standard)
      {
    case ns3::WIFI_PHY_STANDARD_80211a:
      NS_FATAL_ERROR("80211a not supported in STDMA mode");
      break;
    case ns3::WIFI_PHY_STANDARD_80211b:
      NS_FATAL_ERROR("80211b not supported in STDMA mode");
      break;
    case ns3::WIFI_PHY_STANDARD_80211g:
      NS_FATAL_ERROR("80211g not supported in STDMA mode");
      break;
    case ns3::WIFI_PHY_STANDARD_80211_10MHZ:
      NS_FATAL_ERROR("Please use WIFI_PHY_STANDARD_8011p_CCH instead");
      break;
    case ns3::WIFI_PHY_STANDARD_80211_5MHZ:
      NS_FATAL_ERROR("80211 5Mhz not supported in STDMA mode");
      break;
    case ns3::WIFI_PHY_STANDARD_holland:
      NS_FATAL_ERROR("80211 - holland not supported in STDMA mode");
      break;
    case ns3::WIFI_PHY_STANDARD_80211p_CCH:
      Configure80211p_CCH();
      break;
    case ns3::WIFI_PHY_STANDARD_80211p_SCH:
      Configure80211p_SCH();
      break;
    default:
      NS_ASSERT(false);
      break;
      }
  }

  void
  StdmaMac::Configure80211p_CCH(void)
  {
  }

  void
  StdmaMac::Configure80211p_SCH(void)
  {
  }

  ns3::Time
  StdmaMac::GetSlotDuration()
  {
    NS_LOG_FUNCTION_NOARGS();
    ns3::WifiTxVector txVector(m_wifiMode, 1, 0, false, 1, 1, false);
    return m_phy->CalculateTxDuration(m_maxPacketSize, txVector, m_wifiPreamble) + m_guardInterval;
  }

  void
  StdmaMac::SetForwardUpCallback(ns3::Callback<void, ns3::Ptr<ns3::Packet>, ns3::Mac48Address, ns3::Mac48Address> upCallback)
  {
    NS_LOG_FUNCTION (this);
    m_forwardUp = upCallback;
  }

  void
  StdmaMac::SetLinkUpCallback(ns3::Callback<void> linkUp)
  {
    NS_LOG_FUNCTION (this);
    m_linkUp = linkUp;
  }

  void
  StdmaMac::SetLinkDownCallback(ns3::Callback<void> linkDown)
  {
    NS_LOG_FUNCTION (this);
    m_linkDown = linkDown;
  }

  void
  StdmaMac::Receive (ns3::Ptr<ns3::Packet> packet, double rxSnr, ns3::WifiMode txMode, ns3::WifiPreamble preamble)
  {
    NS_LOG_FUNCTION(this << packet << rxSnr << txMode << preamble);

    if (!m_startedUp )
      {
        NS_LOG_DEBUG(ns3::Simulator::Now() << " " << ns3::Simulator::GetContext() << " StdmaMac:Receive() node " << ns3::Simulator::GetContext() << " has not started up yet. Incoming packet is discarded.");
        return;
      }
    if (m_manager->GetStart() > ns3::Simulator::Now())
      {
    	NS_LOG_DEBUG(ns3::Simulator::Now() << " " << ns3::Simulator::GetContext() << " StdmaMac:Receive() node " << ns3::Simulator::GetContext() << " has not started up yet. Incoming packet is discarded.");
		return;
      }

    NS_LOG_DEBUG(ns3::Simulator::Now() << " " << ns3::Simulator::GetContext() << " StdmaMac:Receive() current global slot id " << m_manager->GetGlobalSlotIndexForTimestamp(ns3::Simulator::Now()));

    // Step 1: remove WifiMac header and FCS trailer
    ns3::WifiMacHeader wifiMacHdr;
    packet->RemoveHeader(wifiMacHdr);
    ns3::WifiMacTrailer fcs;
    packet->RemoveTrailer(fcs);

    // Step 2: continue depending on the type of packet that has been received
    if (wifiMacHdr.IsMgt())
      {
        NS_FATAL_ERROR("StdmaMac:Receive() " << m_self << " received a management frame, which is unexpected.");
      }
    if (!wifiMacHdr.GetAddr1().IsGroup())
      {
        NS_FATAL_ERROR("StdmaMac:Receive() " << m_self << " received a unicast packet, but all packets should be delivered to broadcast addresses.");
      }
    if (!wifiMacHdr.IsData())
      {
        NS_FATAL_ERROR("StdmaMac:Receive() " << m_self << " the packet received was not data nor management, which is unexpected.");
      }
    else
      {
        // Try to decode the StdmaHeader...
        StdmaHeader stdmaHdr;
        packet->RemoveHeader(stdmaHdr);
        // If the type id of this header is not a STDMA header
        if (stdmaHdr.GetTypeId() != StdmaHeader::GetTypeId())
          {
            NS_FATAL_ERROR("StdmaMac:Receive() " << m_self << " the packet received did not contain a STDMA header, which is unexpected.");
          }

        // If it had a STDMA header, continue...
        const ns3::Mac48Address to = wifiMacHdr.GetAddr1();
        const ns3::Mac48Address from = wifiMacHdr.GetAddr2();
        NS_LOG_DEBUG(ns3::Simulator::Now() << " " << ns3::Simulator::GetContext() << " StdmaMac:Receive() received a broadcast packet from " << from);
        uint32_t current = m_manager->GetSlotIndexForTimestamp(ns3::Simulator::Now());
        ns3::Vector position(stdmaHdr.GetLatitude(), stdmaHdr.GetLongitude(), 0);
        uint8_t timeout = stdmaHdr.GetTimeout();
        uint16_t offset = stdmaHdr.GetOffset();
        bool networkEntry = stdmaHdr.GetNetworkEntry();
        NS_LOG_DEBUG(ns3::Simulator::Now() << " " << ns3::Simulator::GetContext() << " StdmaMac:Receive() current slot = " << current);
        NS_LOG_DEBUG(ns3::Simulator::Now() << " " << ns3::Simulator::GetContext() << " StdmaMac:Receive() latitude = " << stdmaHdr.GetLatitude());
        NS_LOG_DEBUG(ns3::Simulator::Now() << " " << ns3::Simulator::GetContext() << " StdmaMac:Receive() longitude = " << stdmaHdr.GetLongitude());
        NS_LOG_DEBUG(ns3::Simulator::Now() << " " << ns3::Simulator::GetContext() << " StdmaMac:Receive() timeout = " << (uint32_t) stdmaHdr.GetTimeout());
        NS_LOG_DEBUG(ns3::Simulator::Now() << " " << ns3::Simulator::GetContext() << " StdmaMac:Receive() offset = " << stdmaHdr.GetOffset());

        // ... mark the current slot as externally allocated if the timeout is greater than 0...
        if (timeout > 0)
          {
            NS_LOG_DEBUG(ns3::Simulator::Now() << " " << ns3::Simulator::GetContext() << " StdmaMac:Receive() --> timeout is greater than zero");
            uint32_t next = (current + offset < m_manager->GetSlotsPerFrame()) ? current + offset : current + offset - m_manager->GetSlotsPerFrame();
            NS_LOG_DEBUG(ns3::Simulator::Now() << " " << ns3::Simulator::GetContext() << " StdmaMac:Receive() next = " << next);
            m_manager->MarkSlotAsAllocated(current, stdmaHdr.GetTimeout(), from, position);
            // in this case, the offset further identifies the next expected transmission slot
            m_manager->MarkSlotAsAllocated(next, 1, from, position);
            NS_LOG_DEBUG(ns3::Simulator::Now() << " " << ns3::Simulator::GetContext() << " StdmaMac:Receive() next refers to the global slot id " << m_manager->GetGlobalSlotIndexForTimestamp(ns3::Simulator::Now()) + offset);
          }
        else  // if the timeout is equal to zero we might be in one of the two following cases:
              //  1) the offset references the slot of the same packet number in the next frame,
              //     in this case we will mark it externally allocated for the next frame (i.e. we
              //     expect a transmission to take place in this slot) and, very important, we will
              //     mark the old (i.e. the current) slot as free again
              //     *** I think we should allocate it for a total of 2 frames
              //         -in case it is just allocated for one frame then a collision might occur if in the next frame the other station selects the same slot that we want to use for the next reservation perion (possible if it is acting before us in the next frame)
              //  2) this is a network entry RA-TDMA based packet.
          {
              // We start with the second case...
              if (networkEntry)
                {
                  uint32_t next = (current + offset < m_manager->GetSlotsPerFrame()) ? (current + offset) : (current + offset - m_manager->GetSlotsPerFrame());
                  NS_LOG_DEBUG(ns3::Simulator::Now() << " " << ns3::Simulator::GetContext() << " StdmaMac:Receive() current = " << current);
                  NS_LOG_DEBUG(ns3::Simulator::Now() << " " << ns3::Simulator::GetContext() << " StdmaMac:Receive() offset = " << offset);
                  NS_LOG_DEBUG(ns3::Simulator::Now() << " " << ns3::Simulator::GetContext() << " StdmaMac:Receive() next = " << next);
                  NS_ASSERT(next < m_manager->GetSlotsPerFrame());
                  m_manager->MarkSlotAsAllocated(next, 1, from, position);
                  NS_LOG_DEBUG(ns3::Simulator::Now() << " " << ns3::Simulator::GetContext() << " StdmaMac:Receive() --> this was a network entry packet, next = " << next);
                  NS_LOG_DEBUG(ns3::Simulator::Now() << " " << ns3::Simulator::GetContext() << " StdmaMac:Receive() next refers to the global slot id " << m_manager->GetGlobalSlotIndexForTimestamp(ns3::Simulator::Now()) + offset);
                }
              else
                {
                  NS_LOG_DEBUG(ns3::Simulator::Now() << " " << ns3::Simulator::GetContext() << " StdmaMac:Receive() --> timeout is zero or less, offset = " << offset);
                  uint32_t newSlot;
                  if (current + offset < m_manager->GetSlotsPerFrame())
                    {
                      newSlot = current + offset;
                    }
                  else
                    {
                      newSlot = (current + offset - m_manager->GetSlotsPerFrame());
                      // it can happen in really rare cases that the offset points beyond the next frame as well
                      // in this case we have to substract another slotsPerFrame as well
                      if (newSlot >= m_manager->GetSlotsPerFrame())
                        {
                          newSlot -= m_manager->GetSlotsPerFrame();
                        }
                    }
                  NS_ASSERT(newSlot >= 0);
                  NS_ASSERT(newSlot < m_manager->GetSlotsPerFrame());
                  if (!m_manager->GetSlot(current)->IsInternallyAllocated())
                    {
                      NS_LOG_DEBUG(ns3::Simulator::Now() << " " << ns3::Simulator::GetContext() << " StdmaMac:Receive() marking slot " << current << " as free in the next frame");
                      m_manager->MarkSlotAsFreeAgain(current);
                    }
                  NS_LOG_DEBUG(ns3::Simulator::Now() << " " << ns3::Simulator::GetContext() << " StdmaMac:Receive() marking slot " << newSlot << " as allocated in the next frame");
                  double slotDuration = m_frameDuration.GetSeconds() / m_manager->GetSlotsPerFrame();
                  ns3::Time when = ns3::Simulator::Now() + ns3::Seconds(slotDuration * (offset-1));
                  m_manager->MarkSlotAsAllocated(newSlot, 2, from, position, when);
                  NS_LOG_DEBUG(ns3::Simulator::Now() << " " << ns3::Simulator::GetContext() << " StdmaMac:Receive() newSlot refers to the global slot id " << m_manager->GetGlobalSlotIndexForTimestamp(ns3::Simulator::Now()) + offset);
                }
          }

        m_rxTrace(packet, timeout, offset);

        // ... and finally pass it up the protocol stack
        ForwardUp(packet, from, to);
      }
  }

  void
  StdmaMac::HandleRxStart (ns3::Time duration)
  {
    if (m_startedUp && m_manager->GetStart() <= ns3::Simulator::Now())
      {
        m_rxOngoing = true;
        m_rxStart = ns3::Simulator::Now();
      }
  }

  void
  StdmaMac::HandleRxSuccess (void)
  {
    if (m_startedUp && m_manager->GetStart() <= ns3::Simulator::Now())
      {
        NS_ASSERT(m_rxOngoing);
        m_rxOngoing = false;
      }
  }

  void
  StdmaMac::HandleRxFailure (void)
  {
    if (m_startedUp && m_manager->GetStart() <= ns3::Simulator::Now())
      {
        NS_ASSERT(m_rxOngoing);
        m_rxOngoing = false;
        uint32_t slotAtStart = m_manager->GetSlotIndexForTimestamp(m_rxStart);
        uint32_t slotAtEnd = m_manager->GetSlotIndexForTimestamp(ns3::Simulator::Now());
        for (uint32_t i = slotAtStart; i <= slotAtEnd; i++)
          {
            m_manager->MarkSlotAsBusy(i);
          }
      }
  }

  void
  StdmaMac::HandleMaybeCcaBusyStartNow (ns3::Time duration)
  {
    NS_LOG_FUNCTION_NOARGS();
    if (m_startedUp && m_manager->GetStart() <= ns3::Simulator::Now())
      {
        uint32_t slotAtStart = m_manager->GetSlotIndexForTimestamp(ns3::Simulator::Now());
        // note: we subtract one microseconds in the following line to make sure that we do not hit the border
        // between two slots
        uint32_t slotAtEnd = m_manager->GetSlotIndexForTimestamp(ns3::Simulator::Now() + duration - ns3::MicroSeconds(1));
        NS_LOG_DEBUG(ns3::Simulator::Now() << " " << ns3::Simulator::GetContext() << " StdmaMac:HandleMaybeCcaBusyStartNow() CCA busy period lasts from slot " << slotAtStart << " to slot " << slotAtEnd);

        // We can safely mark all these slots as busy, because in case the transmissions can be decoded successfully,
        // they corresponding reception event will override the status of this/these slots
        for (uint32_t i = slotAtStart; i <= slotAtEnd; i++)
          {
            m_manager->MarkSlotAsBusy(i);
          }
      }
  }

} // namespace stdma
