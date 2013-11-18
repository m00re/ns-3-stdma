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

#include "stdma-mac-helper.h"
#include "ns3/stdma-mac.h"
#include "ns3/pointer.h"
#include "ns3/boolean.h"

namespace stdma {

StdmaMacHelper::StdmaMacHelper ()
{
}

StdmaMacHelper::~StdmaMacHelper ()
{
}

StdmaMacHelper
StdmaMacHelper::Default (void)
{
  StdmaMacHelper helper;
  helper.SetType("stdma::StdmaMac");
  return helper;
}

void
StdmaMacHelper::SetType (std::string type,
                         std::string n0, const ns3::AttributeValue &v0,
                         std::string n1, const ns3::AttributeValue &v1,
                         std::string n2, const ns3::AttributeValue &v2,
                         std::string n3, const ns3::AttributeValue &v3,
                         std::string n4, const ns3::AttributeValue &v4,
                         std::string n5, const ns3::AttributeValue &v5,
                         std::string n6, const ns3::AttributeValue &v6,
                         std::string n7, const ns3::AttributeValue &v7)
{
  m_mac.SetTypeId (type);
  m_mac.Set (n0, v0);
  m_mac.Set (n1, v1);
  m_mac.Set (n2, v2);
  m_mac.Set (n3, v3);
  m_mac.Set (n4, v4);
  m_mac.Set (n5, v5);
  m_mac.Set (n6, v6);
  m_mac.Set (n7, v7);
}

ns3::Ptr<StdmaMac>
StdmaMacHelper::Create (void) const
{
  ns3::Ptr<StdmaMac> mac = m_mac.Create<StdmaMac> ();
  return mac;
}

} //namespace stdma
