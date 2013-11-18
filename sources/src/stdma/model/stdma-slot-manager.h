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

#ifndef STDMA_SLOT_MANAGER_H_
#define STDMA_SLOT_MANAGER_H_

#include "ns3/object.h"
#include "ns3/nstime.h"
#include "ns3/vector.h"
#include "ns3/traced-callback.h"
#include "ns3/mac48-address.h"

#include <map>
#include <set>

namespace stdma {

/**
   * \brief Single slot that is managed by the StdmaSlotManager
   *
   * This class implements the logic / state of a single STDMA slot. A slot can either be free, busy or allocated, whereas
   * a distinction is made whether the slot is externally or internally allocated. A slot is considered to be internally
   * allocated if the station has allocated this slot to itself. The state is maintained as long as the internal timeout
   * value is greater than zero. A slot is considered to be externally allocated if a transmission has been observed in the
   * past that indicated that this slot is going to be used. Note that it is not required that this past transmission has been
   * observed in the same slot. It is also possible that this external allocation has been communicated through the offset
   * parameter in a different slot. The state externally allocated is maintained as long as the recorded timeout value is greater
   * than zero. A slot is considered to be busy if either (a) a packet has been detected in the same slot of the previous frame
   * but the packet failed to be decoded successfully, or if (b) the average energy level at the receiver during the same slot
   * of the previous frame was above the configured clear channel assessment (CCA) threshold. A slot is considered to be free if
   * all of the following conditions hold: (a) no transmission has been observed in the past that indicated that this slot
   * is going to be used in the future; (b) no preamble was detected in this slot; (c) the average energy level at the receiver
   * measured during the same slot of the previous frame was below the configured CCA threshold. The priority of the states is
   * as follows (from lowest to highest): free, busy, allocated, and a state can only be overwritten if the new state is of
   * higher priority.
   *
   * * \ingroup stdma
   */
  class StdmaSlot : public ns3::Object {

  public:

    enum State {
      FREE,             // A slot is said to be free if it is neither used, neither busy
      ALLOCATED,        // A slot is reserved if it is allocated/reserved by another station/node
      BUSY              // A slot is busy if it is not allocated by anybody, but the energy
                        // was above the clear channel assignment (CCA)threshold
    };

    StdmaSlot (uint32_t index);
    virtual ~StdmaSlot();

    StdmaSlot::State GetState();
    bool IsInternallyAllocated();

    void MarkAsFree();
    bool IsFree();
    bool IsFree(ns3::Time until);
    void MarkAsAllocated(uint8_t timeout, ns3::Mac48Address owner, ns3::Vector position, ns3::Time notBefore);
    bool IsAllocated();
    void MarkAsInternallyAllocated(uint8_t timeout);
    void MarkAsBusy();
    bool IsBusy();
    void RebaseIndex(uint32_t index);

    uint32_t GetSlotIndex();
    void SetTimeout(uint8_t t);
    void SetInternalTimeout(uint8_t t);
    uint8_t GetTimeout();
    uint8_t GetInternalTimeout();
    ns3::Vector GetPosition();
    ns3::Mac48Address GetOwner();

  private:

    bool m_internal;
    State m_state;
    State m_previousState;
    uint8_t m_timeout;
    uint8_t m_internalTimeout;
    uint32_t m_index;
    ns3::Vector m_position;
    ns3::Mac48Address m_owner;
    ns3::Time m_notBefore;

  };

  class RandomAccessDetails : public ns3::Object {

    public:

      RandomAccessDetails ();
      virtual ~RandomAccessDetails();

      void SetWhen (ns3::Time when);
      void SetProbability (double p);
      void SetRemainingSlots (uint16_t left);

      ns3::Time GetWhen();
      double GetProbability();
      uint16_t GetRemainingSlots();

    private:

      ns3::Time m_when;
      double m_p;
      uint16_t m_slotsLeft;

    };

  /**
   * \brief The slot manager of a STDMA-based medium access control layer.
   *
   * This class implements the slot (re-)reservation policies of a STDMA-based medium access control layer and
   * allows to abstract from internal details. It can be used to determine the current slot index, choose the
   * nominal start slots, perform a first reservation of slots, and lateron also to perform slot re-reservations.
   *
   * * \ingroup stdma
   */
  class StdmaSlotManager : public ns3::Object {

  public:

    static ns3::TypeId
    GetTypeId (void);

    StdmaSlotManager();
    virtual ~StdmaSlotManager();

    /**
     * Setups a new instance of this class, and sets the starting time of the frame to start, the
     * duration of a single frame to frameDuration, and the number of slots to numSlots.
     *
     * @param start The time at which the underlying first frame starts.
     * @param frameDuration The duration of the underlying frames
     * @param numSlots The number of slots that shall fit into a single frame
     */
    void
    Setup(ns3::Time start, ns3::Time frameDuration, ns3::Time slotDuration, uint32_t candidateSlots);

    /**
     * Returns the time at which the first observed frame has started. Since STDMA assumes that
     * all stations are synchronized on a slot level, this time can only be a multiple of the slot
     * duration
     *
     * @return The starting time of the first frame that is observed by this slot manager
     */
    ns3::Time
    GetStart();

    /**
     * Returns the duration of a STDMAframe
     * @return The duration of a frame
     */
    ns3::Time
    GetFrameDuration();

    /**
     * Returns the number of slots that comprise one super frame
     * @return The number of slots per super frame
     */
    uint32_t
    GetSlotsPerFrame();

    /**
     * Sets the report rate (per super frame) according to which the NSS and NS are selected
     *
     * @param rate The report rate which shall be achieved within one frame
     */
    void SetReportRate(uint32_t rate);

    /**
     * Defines the minimum candidate slot set size. According to the STDMA standard the candidate set should
     * comprise at least 4 candidate slots. This is achieved by adding externally allocated slots if not enough
     * free slots exist anymore.
     *
     * @param size The minimum set size to ensure
     */
    void SetMinimumCandidateSlotSetSize (uint32_t size);

    /**
     * Sets the size of the selection interval relative to the size of the nominal increment (NI),
     * such that the number of slots in the interval [NS-SI/2, NS+SI/2] equals ceil(NI*ratio)
     *
     * @param ratio The relative size (w.r.t. the nominal increment) of the selection interval
     */
    void SetSelectionIntervalRatio(double ratio);

    /**
     * This method should be called only once, according to the standard during network entry. It is responsible
     * for the selection of the nominal start slots, which pre-determine the location of the selection intervals
     * inside of a frame. In the current implementation this method is called in StdmaMac::PerformNetworkEntry
     */
    void SelectNominalSlots();

    /**
     * Returns the reservation index of the current/next packet (within the frame) to be transmitted, and
     * automatically increases the internal counter to the next reservation.
     * @return The reservation index of the current/next packet to be transmitted
     */
    uint32_t GetCurrentReservationNo();

    /**
     * Instructs the slot manager to select a nominal transmission slot (NTS) for the n-th packet reservation
     * of the current super frame. This shall only be successful if the packet for this reservation has not been
     * transmitted yet in the current frame and if the NTS has not already been selected previously. Hence, the
     * method is called in StdmaMac::DoTransmit but only if the parameter 'firstFrame' of this method is set to
     * true.
     *
     * @param n The index of the reservation for which a slot shall be selected
     * @param timeout The number of frames this reservation shall be kept
     */
    void SelectTransmissionSlotForReservationWithNo (uint32_t n, uint8_t timeout);

    /**
     * Performs a re-reservation for the n-th reservation index. Such a re-reservation takes place of the timeout
     * of a reservation expires. Hence it is necessary that a reservation already exists. The return value indicates
     * the offset (in terms of number of slots) between the slot previously reserved an the slot reserved in the next
     * frame.
     *
     * \param n The index of the reservation for which a slot shall be selected
     * \param timeout timeout The number of frames this reservation shall be kept
     * \return The offset between the previously selected slot and the newly selected slot (>= 0 and < number of slots per frame)
     */
    uint32_t ReSelectTransmissionSlotForReservationWithNo (uint32_t n, uint8_t timeout);

    /**
     * Return the scheduled transmission time for the reservation with index n. This method
     * is typically used by the StdmaMac implementation to schedule the next transmission event
     * in the simulator.
     *
     * @param n The index of the reservation for which the scheduled transmission time is requested
     */
    ns3::Time GetTimeUntilTransmissionOfReservationWithNo (uint32_t n);

    /**
     * Returns the slot index for a given timestamp
     * @param t The timestamp for which the slot index shall be determined
     */
    uint32_t GetSlotIndexForTimestamp (ns3::Time t);

    /**
     * Calculates the offset (i.e. the number of slots) between two arbitrary packet reservations.
     * The reservations are referenced through their local number within the frame (e.g. first own packet
     * reservation within the frame has number 0, second packet reservation has number 1, ...).
     *
     * @param k The reservation number of the first packet
     * @param l The reservation number of the second packet
     * @return The offset in number of slots between these two packets
     */
    uint32_t CalculateSlotOffsetBetweenTransmissions(uint32_t k, uint32_t l);

    /**
     * Determines whether a packet number needs re-reservation or not
     *
     * @param n The reservation number for which to check
     * @return Whether this packet number needs to be re-reserved or not
     */
    bool NeedsReReservation (uint32_t n);

    /**
     * Decreases the stored timeout value of the packet reservation with number n.
     * @param n The number of the packet reservation for which to decrease the timeout
     * @return The timeout value after the decrement
     */
    uint8_t DecreaseTimeOutOfReservationWithNumber(uint32_t n);

    /**
     * Marks the slot with the given index as externally reserved.
     * @param index The slot index
     * @param timeout The number of subsequent frames during which this slot will be used
     * @param node The MAC address of the node who owns this slot
     * @param position The position of the owner of this slot
     */
    void MarkSlotAsAllocated(uint32_t index, uint8_t timeout, ns3::Mac48Address node, ns3::Vector position);

    /**
     * Marks the slot with the given index as externally reserved.
     * @param index The slot index
     * @param timeout The number of subsequent frames during which this slot will be used
     * @param node The MAC address of the node who owns this slot
     * @param position The position of the owner of this slot
     * @param notbefore Don't treat as allocated before this timestamp
     */
    void MarkSlotAsAllocated(uint32_t index, uint8_t timeout, ns3::Mac48Address node, ns3::Vector position, ns3::Time notbefore);

    /**
     * Marks the slot with the given index as free again.
     * @param index The slot index
     */
    void MarkSlotAsFreeAgain(uint32_t index);

    /**
     * Marks the slot with the given index as busy
     * @param index The slot index
     */
    void MarkSlotAsBusy(uint32_t index);

    /**
     * Tells the slot manager to adjust its frame starting and end times such that the frame
     * is starting at the provided time stamp. All slot numbers are re-arranged accordingly.
     */
    void RebaseFrameStart(ns3::Time now);

    /**
     * Calculates the time stamp of the random access based network entry transmission, based
     * on the number of slots remaining in the network entry entry phase, and the current probability
     * level.
     */
    ns3::Ptr<RandomAccessDetails> GetNetworkEntryTimestamp(uint32_t remainingSlots, double p);

    /**
     * Determines whether there are still free slots left among the next N slots
     * @param remainingSlots The number of slots to check from now on
     */
    bool HasFreeSlotsLeft(uint32_t remainingSlots);

    /**
     * Takes the current slot and checks whether it is still marked as free
     */
    bool IsCurrentSlotStillFree();

    ns3::Ptr<StdmaSlot> GetSlot(uint32_t index);

    uint64_t GetGlobalSlotIndexForTimestamp(ns3::Time t);

  private:
    /**
     * Updates the internal reservation table and marks the status of each slot according to its
     * new status when going from one frame to another. This method is called internally whenever
     * the beginning of the next frame is detected.
     */
    void UpdateSlotObservations();

    ns3::Time GetTimeForSlotIndex(uint32_t index);

    std::map<uint32_t, ns3::Ptr<StdmaSlot> > m_slots;
    std::map<uint32_t, ns3::Ptr<StdmaSlot> > m_selections;
    std::map<ns3::Mac48Address, uint32_t> m_collisions;

    ns3::Time m_start;
    ns3::Time m_frameDuration;
    ns3::Time m_slotDuration;
    uint32_t m_numSlots;
    uint32_t m_rate;
    uint32_t m_ni;
    uint32_t m_siHalf;

    std::vector<uint32_t> m_nss;        // Nominal start slots
    ns3::Time m_lastFrameStart;
    uint32_t m_current;

    uint32_t m_mininumCandidates;

    ns3::TracedCallback<std::vector<uint32_t> > m_nominalSlotTrace;
    ns3::TracedCallback<uint32_t, uint32_t, bool> m_reservationTrace;
    ns3::TracedCallback<uint32_t, uint32_t, bool, bool> m_reReservationTrace;

    friend class StdmaSlotManagerTest;
  };

} // namespace stdma

#endif /* STDMA_SLOT_MANAGER_H_ */
