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

#ifndef STDMA_HEADER_H_
#define STDMA_HEADER_H_

#include "ns3/header.h"
#include "ns3/buffer.h"

namespace stdma {

/**
 * \brief Defines the contents of a STDMA packet header
 *
 * This class defines the contents of a STDMA packet header, which includes GPS coordinates
 * (latitude and longitude), offset information to the next transmission slot to be used, and
 * the timeout value of the reservation duration.
 *
 * \ingroup stdma
 */
class StdmaHeader : public ns3::Header
{

public:

  static ns3::TypeId GetTypeId (void);
  StdmaHeader ();
  virtual ~StdmaHeader ();
  virtual ns3::TypeId GetInstanceTypeId (void) const;
  virtual uint32_t GetSerializedSize (void) const;
  virtual void Serialize (ns3::Buffer::Iterator start) const;
  virtual uint32_t Deserialize (ns3::Buffer::Iterator start);
  virtual void Print (std::ostream &os) const;

  /**
   * \param lat The latitude of the position from which the corresponding packet was sent
   */
  void SetLatitude(double lat);
  double GetLatitude();
  /**
   * \param lon The longitude of the position from which the corresponding packet was sent
   */
  void SetLongitude(double lon);
  double GetLongitude();
  /**
   * \param offset The number of slots to the next transmission slot which this node will use
   */
  void SetOffset(uint16_t offset);
  uint16_t GetOffset();
  /**
   * \param timeout The number of future frames over which the same slot will be re-used
   */
  void SetTimeout(uint8_t timeout);
  uint8_t GetTimeout();
  /**
   * Enable a flag that indicates that this packet is a network entry packet.
   */
  void SetNetworkEntry();
  bool GetNetworkEntry();

private:

  double m_latitude;
  double m_longitude;

  uint16_t m_offset;  // This represents the offset to the next scheduled transmission
                      // whenever the timeout is greater than zero. If the timeout is zero
                      // the offset represents the offset to the new slot in the next frame

  uint8_t m_timeout;  // Describes the slot timeout and defines the number of future frames
                      // during which this slot will be used

  uint8_t m_entry;    // Used as a flag to indicate that this is a network entry packet

};

} // namespace stdma

#endif /* STDMA_HEADER_H_ */
