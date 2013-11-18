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

#ifndef STMDA_MAC_H
#define STDMA_MAC_H

#include "ns3/packet.h"
#include "ns3/mac48-address.h"
#include "ns3/wifi-mac-header.h"
#include "ns3/event-id.h"
#include "ns3/random-variable.h"
#include "ns3/vector.h"
#include "ns3/wifi-mac-queue.h"
#include "stdma-slot-manager.h"

#include "ns3/wifi-phy.h"
#include "ns3/ssid.h"
#include "ns3/qos-utils.h"

namespace stdma {

  /**
   * \defgroup stdma Self-Organizing TDMA
   *
   * This section documents the API of the ns-3 module that implements Self-organizing TDMA (STDMA)for inter-vehicle communication networks. The implementation
   * is a replacement of the ns3::WifiMac class and can be to compare CSMA-based periodic broadcasting in VANETs against STDMA-based periodic broadcasting in VANETs.
   *
   * Self-organized TDMA is an alternative to CSMA and was invented by H{\aa}kan Lans. STDMA employs a reservation scheme where nodes announce their next
   * transmissions, resulting in less random elements compared to CSMA. The STDMA approach has been standardized and is used both in the Automatic Identification
   * System (AIS) as well as in VDL Mode 4 to exchange positioning information between ships and aircrafts, respectively.
   */
  class StdmaMac;

  /**
   * Listening class for phy events and forwards them to the StdmaMac implementation
   */
  class StdmaMacPhyListener : public ns3::WifiPhyListener
  {
  public:
    StdmaMacPhyListener(StdmaMac *mac);
    virtual
    ~StdmaMacPhyListener();

    virtual void
    NotifyRxStart(ns3::Time duration);
    virtual void
    NotifyRxEndOk(void);
    virtual void
    NotifyRxEndError(void);
    virtual void
    NotifyTxStart(ns3::Time duration);
    virtual void
    NotifyMaybeCcaBusyStart(ns3::Time duration);
    virtual void
    NotifySwitchingStart(ns3::Time duration);
  private:
    StdmaMac *m_StdmaMac;
  };

  /**
   * \brief An implementation of the MAC protocol Self-Organizing TDMA for the usage in 802.11p environments.
   *
   * This class encapsulates all the STDMA MAC functionality for periodic broadcasting. The STDMA medium access control
   * protocol is described in ITU-R M.1371-4 "Technical Characteristics for an Automatic Identification System Using Time Division
   * Multiple Access in the VHF Maritime Mobile Band", 2010. The protocol employs a TDMA-based approach and divides the time
   * in so called frames that last for a certain duration, and those frames into equally sized transmission slots that
   * accommodate a single packet transmission. The lifetime of a station in STDMA is divided into four different phases:
   * (1) initialization, (2) network entry, (3) first frame, and (4) continuous operation. These phases ensure that each station
   * first obtains an understanding of the slot allocation status, then announces its presence to the network, and finally
   * performs the initial slot allocation for all transmissions to be made during one frame. Afterwards, the continuous operation
   * phase is entered in which only slot re-allocations are carried out.
   *
   * It is required to use a report rate > 1 Hz to achieve valid operation. Unicast packets
   * are also not supported and simply "ignored" at the time being.
   *
   * \ingroup stdma
   */
  class StdmaMac : public ns3::Object
  {
    friend class StdmaHelper;
  public:
    static ns3::TypeId
    GetTypeId (void);

    StdmaMac ();
    virtual ~StdmaMac ();

    ns3::Time
    GetGuardInterval (void) const;

    void
    SetGuardInterval (const ns3::Time gi);

    void
    SetSelectionIntervalRatio (double ratio);

    double
    GetSelectionIntervalRatio () const;

    void
    SetMinimumCandidateSetSize (uint32_t size);

    void
    SetAddress (ns3::Mac48Address address);

    ns3::Mac48Address
    GetAddress (void) const;

    void
    SetSsid (ns3::Ssid ssid);

    ns3::Ssid
    GetSsid (void) const;

    void
    SetBssid (ns3::Mac48Address bssid);

    ns3::Mac48Address
    GetBssid (void) const;

    /**
     * Is supposed to enqueue a unicast packet into the transmission queue, but as this MAC implementation
     * supports only broadcast packets, the packets is simply discarded/ignored.
     *
     * \param packet The packet that should be transmitted
     * \param to The destination MAC address of the packet
     * \param from The source MAC address of this packet
     */
    void
    Enqueue (ns3::Ptr<const ns3::Packet> packet, ns3::Mac48Address to, ns3::Mac48Address from);

    /**
	 * Enqueues a broadcast packet into the transmission queue. This packet will be transmitted in one
	 * of the slots reserved by STDMA.
	 *
	 * \param packet The packet that should be transmitted
	 * \param to The destination MAC address of the packet
	 */
    void
    Enqueue (ns3::Ptr<const ns3::Packet> packet, ns3::Mac48Address to);

    /**
     * This method starts up the STDMA logic and triggers the start of the initialization phase
     * as defined in the standard. It is typically scheduled by StdmaHelper::Install when connecting
     * all protocol layers and network devices together.
     */
    void
    StartInitializationPhase ();

    /**
     * Will always return false in the current implementation as the feature is not supported.
     */
    bool
    SupportsSendFrom (void) const;

    void
    SetWifiPhy (ns3::Ptr<ns3::WifiPhy> phy);

    /**
     * Sets the callback function exposed by the layer to which received packets shall be forwarded. This is typically
     * a StdmaNetDevice and attached by the WifiPhyHelper.
     *
     * \param upCallback The reference to the object and the method to call whenever a packet is successfully received
     */
    void
    SetForwardUpCallback (ns3::Callback<void, ns3::Ptr<ns3::Packet>, ns3::Mac48Address, ns3::Mac48Address> upCallback);

    /**
     * Sets the callback function exposed by the layer to which a notification shall be sent whenever the link is up.
     * This is typically a StdmaNetDevice and attached by the WifiPhyHelper.
     *
     * \param linkUp The reference to the object and the method to call whenever a "link" is established
     */
    void
    SetLinkUpCallback (ns3::Callback<void> linkUp);

    /**
	 * Sets the callback function exposed by the layer to which a notification shall be sent whenever the link is down.
	 * This is typically a StdmaNetDevice and attached by the WifiPhyHelper.
	 *
	 * \param linkUp The reference to the object and the method to call whenever the "link" gets down.
	 */
    void
    SetLinkDownCallback (ns3::Callback<void> linkDown);

    /**
     * This method is called whenever the physical layer successfully receives a message and passes it up to
     * the medium access control layer.
     *
     * \param packet	The packet which was successfully received by the physical layer
     * \param rxSnr		The signal-to-noise ratio (SNR) of the received packet
     * \param txMode	The WIFI mode that was used to transmit the packet
     * \param preamble	The type of preamble used by the transmitter for this packet
     */
    void
    Receive (ns3::Ptr<ns3::Packet> packet, double rxSnr, ns3::WifiMode txMode, ns3::WifiPreamble preamble);

    /**
     * This method is called whenever the reception process at physical layer is started. It does not pass the packet
     * yet, but informs the medium access control layer about the current state of the physical layer and the
     * estimated duration of this state.
     *
     * @param duration The expected duration the physical layer will remain in this state
     */
    void
    HandleRxStart (ns3::Time duration);

    /**
     * This method is called whenever the reception of a packet at the physical layer completed successfully. It does
     * not pass the packet yet (this is handled by StdmaMac::Receive) but informs about the state change that occurred
     * at the physical layer.
     */
    void
    HandleRxSuccess (void);

    /**
     * This method is called whenever the reception of a packet at the physical layer did not complete successfully.
     */
    void
    HandleRxFailure (void);

    /**
     * This method is called whenever the physical layer enters a CCA busy period and allows the medium access control
     * layer to act accordingly.
     *
     * @param duration The expected duration the physical layer will remain in this state
     */
    void
    HandleMaybeCcaBusyStartNow (ns3::Time duration);

    /**
     * Configures the medium access control layer according to the WifiStandard provided to this method. The current
     * implementation only supports the standards ns3::WIFI_PHY_STANDARD_80211p_CCH and WIFI_PHY_STANDARD_80211p_SCH
     * which are dedicated to inter-vehicle communication networks.
     *
     * \param standard The Wifi standard according to which the MAC layer shall be configured
     */
    void
    ConfigureStandard(enum ns3::WifiPhyStandard standard);

  protected:

    /**
     * This method takes care of packet forwarding to higher layers and encapsulates all details
     * that go along with this activity.
     */
    void
    ForwardUp (ns3::Ptr<ns3::Packet> packet, ns3::Mac48Address from, ns3::Mac48Address to);

    ns3::Ptr<ns3::WifiPhy> m_phy;
    ns3::Ptr<ns3::WifiMacQueue> m_queue;

    ns3::Callback<void, ns3::Ptr<ns3::Packet>, ns3::Mac48Address, ns3::Mac48Address> m_forwardUp;
    ns3::Callback<void> m_linkUp;
    ns3::Callback<void> m_linkDown;
    ns3::Mac48Address m_self;
    ns3::Mac48Address m_bssid;
    ns3::Ssid m_ssid;

  private:

    void
    Configure80211p_CCH (void);
    void
    Configure80211p_SCH (void);

    ns3::Time
    GetSlotDuration();

    /**
     * This method is scheduled exactly one super frame after the initialization phase has been started.
     * The medium access control layer then has listened to the channel for a one frame and has a full
     * understanding (at least in theory) what is going on in the network and which slots are already reserved
     * or busy.
     */
    void
    EndOfInitializationPhase ();

    /**
     * Performs the network entry and transmits the first packet in order to announce the own presence in
     * the network. This method will also pre-announce the next transmission slot and re-base the frame start
     * such that the frame starts with the next slot.
     */
    void
    PerformNetworkEntry (uint32_t remainingSlots, double p);

    /**
     * There is only one function that handles transmissions, and that is this one here. New transmission
     * events are either scheduled at the end of the initialization phase, or after having transmitted
     * a packet. Hence, new events are either scheduled from within StdmaMac::PerformNetworkEntry or from
     * StdmaMac::DoTransmit
     */
    void
    DoTransmit(bool firstFrame);

    double m_selectionIntervalRatio;
    uint32_t m_minimumCandidateSetSize;
    ns3::Time m_guardInterval;
    ns3::WifiPreamble m_wifiPreamble;
    ns3::WifiMode m_wifiMode;
    uint8_t m_reportRate;
    ns3::Time m_frameDuration;
    uint32_t m_maxPacketSize;
    ns3::RandomVariable m_timeoutRng;
    uint16_t m_slotsForRtdma;

    StdmaMacPhyListener *m_phyListener;
    ns3::Ptr<StdmaSlotManager> m_manager;
    ns3::EventId m_endInitializationPhaseEvent;
    ns3::EventId m_nextTransmissionEvent;

    bool m_rxOngoing;
    ns3::Time m_rxStart;
    bool m_startedUp;

    ns3::TracedCallback<ns3::Time, ns3::Time, ns3::Time> m_startupTrace;
    ns3::TracedCallback<ns3::Ptr<const ns3::Packet>, ns3::Time, bool> m_networkEntryTrace;
    ns3::TracedCallback<ns3::Ptr<const ns3::Packet>, uint32_t, uint8_t, uint32_t> m_txTrace;
    ns3::TracedCallback<ns3::Ptr<const ns3::Packet>, uint8_t, uint32_t> m_rxTrace;

  };

} // namespace stdma

#endif /* STDMA_MAC_H */
