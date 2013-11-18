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

#ifndef STDMA_HELPER_H
#define STDMA_HELPER_H

#include <string>
#include <vector>
#include "ns3/attribute.h"
#include "ns3/object-factory.h"
#include "ns3/node-container.h"
#include "ns3/net-device-container.h"
#include "ns3/wifi-phy-standard.h"
#include "ns3/wifi-helper.h"
#include "ns3/trace-helper.h"
#include "ns3/wifi-phy.h"
#include "stdma-mac-helper.h"

namespace stdma {


/**
 * \brief helps to create StdmaNetDevice objects
 *
 * This class can help to create a large set of similar StdmaNetDevice objects
 * and to configure a large set of their attributes during creation.
 */
class StdmaHelper
{
public:
  /**
   * Create an Stdma helper in an empty state: all its parameters
   * must be set before calling ns3::StdmaHelper::Install
   */
  StdmaHelper ();

  /**
   * \returns a new StdmaHelper in a default state
   *
   * The default state is defined as being a broadcast only mechanism and a PHY compatible
   * with 802.11g
   */
  static StdmaHelper Default (void);

  /**
   * \param phy The PHY helper to create PHY objects
   * \param mac The stdma MAC helper to create stdma MAC objects
   * \param c The set of nodes on which an stdma device must be created
   * \param startups The set of startup times for all nodes (using the same ordering
   *                 as in the node container set
   * \returns A device container which contains all the devices created by this method.
   */
  ns3::NetDeviceContainer Install (const ns3::WifiPhyHelper &phy,
                                   const StdmaMacHelper &mac, ns3::NodeContainer c, std::vector<ns3::Time> startups) const;

  /**
   * \param phy The PHY helper to create PHY objects
   * \param mac The stdma MAC helper to create stdma MAC objects
   * \param c The set of nodes on which an stdma device must be created
   * \returns A device container which contains all the devices created by this method.
   */
  ns3::NetDeviceContainer Install (const ns3::WifiPhyHelper &phy,
                                   const StdmaMacHelper &mac, ns3::NodeContainer c) const;
  /**
   * \param phy The PHY helper to create PHY objects
   * \param mac The Stdma MAC helper to create Stdma MAC objects
   * \param node The node on which a wifi device must be created
   * \returns A device container which contains all the devices created by this method.
   */
  ns3::NetDeviceContainer Install (const ns3::WifiPhyHelper &phy,
                                   const StdmaMacHelper &mac, ns3::Ptr<ns3::Node> node) const;
  /**
   * \param phy The PHY helper to create PHY objects
   * \param mac The Stdma MAC helper to create Stdma MAC objects
   * \param nodeName The name of node on which a wifi device must be created
   * \returns A device container which contains all the devices created by this method.
   */
  ns3::NetDeviceContainer Install (const ns3::WifiPhyHelper &phy,
                                   const StdmaMacHelper &mac, std::string nodeName) const;

  /**
   * \param standard the phy standard to configure during installation
   *
   * By default, all objects are configured for 802.11a
   */
  void SetStandard (enum ns3::WifiPhyStandard standard);


private:

  enum ns3::WifiPhyStandard m_standard;

};

} // namespace stdma

#endif /* STDMA_HELPER_H */
