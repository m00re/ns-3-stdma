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

#ifndef TWO_NODES_TEST_H_
#define TWO_NODES_TEST_H_

#include "ns3/log.h"
#include "ns3/test.h"
#include "ns3/nstime.h"
#include "ns3/ptr.h"
#include "ns3/packet.h"

namespace stdma {

class StdmaTwoNodesTest : public ns3::TestCase
{
public:
  StdmaTwoNodesTest ();

  virtual void DoRun (void);
  void StdmaTxTrace (std::string context, ns3::Ptr<const ns3::Packet> p, uint32_t no, uint8_t timeout, uint32_t offset);
  void StdmaRxTrace (std::string context, ns3::Ptr<const ns3::Packet> p, uint8_t timeout, uint32_t offset);
  void StdmaStartupTrace (std::string context, ns3::Time when, ns3::Time frameDuration, ns3::Time slotDuration);
  void StdmaNetworkEntry (std::string context, ns3::Ptr<const ns3::Packet> p, ns3::Time offset);

private:
  uint32_t GetProtocolOverheads();
  uint32_t GetGlobalSlotId(ns3::Time t);

  uint32_t m_count;
  uint32_t m_nextTxNodeOne;
  uint32_t m_nextTxNodeTwo;
  uint32_t m_nextRxFromNodeOne;
  uint32_t m_nextRxFromNodeTwo;
  ns3::Time m_slotDuration;

};

} // namespace stdma

#endif /* TWO_NODES_TEST_H_ */
