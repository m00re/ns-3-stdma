/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2013 Jens Mittag
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
 * Author: Jens Mittag <jens.mittag@kit.edu>
 *
 */

#include "ns3/core-module.h"
#include "ns3/wifi-module.h"
#include "ns3/stdma-module.h"
#include "ns3/mobility-module.h"
#include "ns3/packet-socket-helper.h"
#include "ns3/packet-socket-address.h"
#include "ns3/on-off-helper.h"
#include "ns3/llc-snap-header.h"
#include "ns3/mac48-address.h"
#include "slot-manager-test.h"

using namespace ns3;

namespace stdma {


  StdmaSlotManagerTest::StdmaSlotManagerTest ()
    : ns3::TestCase ("StdmaSlotManagerTest")
  {
  }

  void
  StdmaSlotManagerTest::DoRun (void)
  {
    ns3::Time start = ns3::Seconds(1.0);
    ns3::Time frameDuration = ns3::Seconds(1.0);
    ns3::Time slotDuration = ns3::NanoSeconds(566000.0);
    ns3::Ptr<StdmaSlotManager> manager = Create<StdmaSlotManager>();
    manager->Setup(start, frameDuration, slotDuration, 4);
    frameDuration = manager->GetFrameDuration();
    NS_TEST_EXPECT_MSG_EQ (0, frameDuration.GetNanoSeconds() % slotDuration.GetNanoSeconds(), "There should be no rest when dividing the frame duration by the slot duration");
    uint32_t numSlots = frameDuration.GetNanoSeconds() / slotDuration.GetNanoSeconds();
    NS_TEST_EXPECT_MSG_EQ (ns3::NanoSeconds(1000122000.0), manager->GetStart(), "The start of the frame should be as expected.");
    start = manager->GetStart();

    for (uint32_t index = 0; index < numSlots; index++)
      {
        NS_TEST_EXPECT_MSG_EQ (StdmaSlot::FREE, manager->GetSlot(index)->GetState(), "All slots should be free in the beginning.");
      }
    manager->MarkSlotAsBusy(20);
    NS_TEST_EXPECT_MSG_EQ (StdmaSlot::BUSY, manager->GetSlot(20)->GetState(), "The slot should be busy as we have marked it busy in the last instruction.");
    ns3::Mac48Address owner("ab:cd:ef:12:34:56");
    manager->MarkSlotAsAllocated(30, 8, owner, ns3::Vector(1.0, 0.0, 0.0), ns3::Seconds(0.0));
    NS_TEST_EXPECT_MSG_EQ (StdmaSlot::ALLOCATED, manager->GetSlot(30)->GetState(), "The slot should be allocated as we have marked it busy in the last instruction.");
    NS_TEST_EXPECT_MSG_EQ (8, manager->GetSlot(30)->GetTimeout(), "The slot should have a timeout of 8 as we have marked it busy in the last instruction.");

    ns3::Time t1 = start + ns3::NanoSeconds(slotDuration.GetNanoSeconds() * 27);
    ns3::Time t2 = start + ns3::NanoSeconds(slotDuration.GetNanoSeconds() * 27.5);
    NS_TEST_EXPECT_MSG_EQ (27, manager->GetSlotIndexForTimestamp(t1), "The slot index for the given time stamp t1 should be 27");
    NS_TEST_EXPECT_MSG_EQ (27, manager->GetSlotIndexForTimestamp(t2), "The slot index for the given time stamp t2 should be 27");

    uint32_t shift = 12;
    ns3::Time newStart = manager->GetStart() + ns3::NanoSeconds(shift * slotDuration.GetNanoSeconds());
    manager->RebaseFrameStart(newStart);
    NS_TEST_EXPECT_MSG_EQ (15, manager->GetSlotIndexForTimestamp(t1), "After rebasing the slot index for the given time stamp t1 should be 15");
    NS_TEST_EXPECT_MSG_EQ (15, manager->GetSlotIndexForTimestamp(t2), "After rebasing the slot index for the given time stamp t2 should be 15");

    NS_TEST_EXPECT_MSG_EQ (StdmaSlot::BUSY, manager->GetSlot(8)->GetState(), "The slot should still be busy after rebase as only the index should have changed.");
    NS_TEST_EXPECT_MSG_EQ (StdmaSlot::ALLOCATED, manager->GetSlot(18)->GetState(), "The slot should still be allocated after rebase as only the index should have changed.");
    NS_TEST_EXPECT_MSG_EQ (8, manager->GetSlot(18)->GetTimeout(), "The slot should still have a timeout of 8 after rebase as only the index should have changed.");

    manager->UpdateSlotObservations();
    NS_TEST_EXPECT_MSG_EQ (StdmaSlot::BUSY, manager->GetSlot(8)->GetState(), "The slot should still be busy after updating slot observations.");
    NS_TEST_EXPECT_MSG_EQ (StdmaSlot::ALLOCATED, manager->GetSlot(18)->GetState(), "The slot should still be allocated after updating slot observations.");
    NS_TEST_EXPECT_MSG_EQ (7, manager->GetSlot(18)->GetTimeout(), "The slot timeout should now be 7 after updating slot observations.");

    manager->UpdateSlotObservations();
    NS_TEST_EXPECT_MSG_EQ (StdmaSlot::FREE, manager->GetSlot(8)->GetState(), "After a second count down this slot should be free again.");
    NS_TEST_EXPECT_MSG_EQ (StdmaSlot::ALLOCATED, manager->GetSlot(18)->GetState(), "The slot should still be allocated after updating slot observations.");
    NS_TEST_EXPECT_MSG_EQ (6, manager->GetSlot(18)->GetTimeout(), "The slot timeout should now be 6 after updating slot observations.");

  }

} // namespace stdma
