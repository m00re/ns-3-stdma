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

#include "stdma-header.h"
#include "ns3/log.h"

NS_LOG_COMPONENT_DEFINE ("StdmaHeader");

namespace stdma {

  ns3::TypeId
  StdmaHeader::GetTypeId (void)
  {
    static ns3::TypeId tid = ns3::TypeId("stdma::StdmaHeader")
        .SetParent<ns3::Header>()
        .AddConstructor<StdmaHeader>();
    return tid;
  }

  StdmaHeader::StdmaHeader ()
    : m_latitude(0),
      m_longitude(0),
      m_offset(0),
      m_timeout(0),
      m_entry(0)
  {
  }

  StdmaHeader::~StdmaHeader ()
  {
  }

  ns3::TypeId
  StdmaHeader::GetInstanceTypeId (void) const
  {
    return GetTypeId();
  }

  uint32_t
  StdmaHeader::GetSerializedSize (void) const
  {
    return sizeof(m_latitude) + sizeof(m_longitude) + sizeof(m_offset) + sizeof(m_timeout) + sizeof(m_entry);
  }

  void
  StdmaHeader::Serialize (ns3::Buffer::Iterator start) const
  {
    ns3::Buffer::Iterator i = start;
    i.WriteU32(m_latitude);
    i.WriteU32(m_longitude);
    i.WriteU16(m_offset);
    i.WriteU8(m_timeout);
    i.WriteU8(m_entry);
  }

  uint32_t
  StdmaHeader::Deserialize (ns3::Buffer::Iterator start)
  {
    m_latitude = start.ReadU32();
    m_longitude = start.ReadU32();
    m_offset = start.ReadU16();
    m_timeout = start.ReadU8();
    m_entry = start.ReadU8();
    return sizeof(m_latitude) + sizeof(m_longitude) + sizeof(m_offset) + sizeof(m_timeout) + sizeof(m_entry);
  }

  void
  StdmaHeader::Print (std::ostream &os) const
  {
    std::string type;
    os << "STDMA Header: (Lat: " << m_latitude
       << ", Lon: " << m_longitude
       << ", Offset: " << m_offset
       << ", Timeout: " << m_timeout << ")";
  }

  void
  StdmaHeader::SetLatitude(double lat)
  {
    m_latitude = lat;
  }

  double
  StdmaHeader::GetLatitude()
  {
    return m_latitude;
  }

  void
  StdmaHeader::SetLongitude(double lon)
  {
    m_longitude = lon;
  }

  double
  StdmaHeader::GetLongitude()
  {
    return m_longitude;
  }

  void
  StdmaHeader::SetOffset(uint16_t offset)
  {
    m_offset = offset;
  }

  uint16_t
  StdmaHeader::GetOffset()
  {
    return m_offset;
  }

  void
  StdmaHeader::SetTimeout(uint8_t timeout)
  {
    m_timeout = timeout;
  }

  uint8_t
  StdmaHeader::GetTimeout()
  {
    return m_timeout;
  }

  void
  StdmaHeader::SetNetworkEntry()
  {
    m_entry = 1;
  }

  bool
  StdmaHeader::GetNetworkEntry()
  {
    return (m_entry > 0);
  }

} // namespace stdma
