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

#ifndef STDMA_MAC_HELPER_H
#define STDMA_MAC_HELPER_H

#include "ns3/object-factory.h"

namespace stdma
{

class StdmaMac;

/**
 * \brief Create STDMA MAC layers for a ns3::StdmaNetDevice.
 *
 * This class can create MACs of type ns3::StdmaMac
 */
class StdmaMacHelper
{
public:
  /**
   * Create a StdmaMacHelper to make life easier for people who want to create STDMA mac layer
   * instances
   */
  StdmaMacHelper();

  /**
   * \internal
   * Destroy a StdmaMacHelper.
   */
  virtual
  ~StdmaMacHelper();

  /**
   * Create a STDMA mac helper in a default working state.
   */
  static StdmaMacHelper
  Default(void);
  /**
   * \param type the type of ns3::StdmaMac to create. -- currently only one time
   * \param n0 the name of the attribute to set
   * \param v0 the value of the attribute to set
   * \param n1 the name of the attribute to set
   * \param v1 the value of the attribute to set
   * \param n2 the name of the attribute to set
   * \param v2 the value of the attribute to set
   * \param n3 the name of the attribute to set
   * \param v3 the value of the attribute to set
   * \param n4 the name of the attribute to set
   * \param v4 the value of the attribute to set
   * \param n5 the name of the attribute to set
   * \param v5 the value of the attribute to set
   * \param n6 the name of the attribute to set
   * \param v6 the value of the attribute to set
   * \param n7 the name of the attribute to set
   * \param v7 the value of the attribute to set
   *
   * All the attributes specified in this method should exist
   * in the requested mac.
   */
  void
  SetType(std::string type, std::string n0 = "", const ns3::AttributeValue &v0 =
          ns3::EmptyAttributeValue(), std::string n1 = "", const ns3::AttributeValue &v1 =
          ns3::EmptyAttributeValue(), std::string n2 = "", const ns3::AttributeValue &v2 =
          ns3::EmptyAttributeValue(), std::string n3 = "", const ns3::AttributeValue &v3 =
          ns3::EmptyAttributeValue(), std::string n4 = "", const ns3::AttributeValue &v4 =
          ns3::EmptyAttributeValue(), std::string n5 = "", const ns3::AttributeValue &v5 =
          ns3::EmptyAttributeValue(), std::string n6 = "", const ns3::AttributeValue &v6 =
          ns3::EmptyAttributeValue(), std::string n7 = "", const ns3::AttributeValue &v7 =
          ns3::EmptyAttributeValue());

  /**
   * \internal
   * \returns a newly-created MAC object.

   */
  virtual ns3::Ptr<StdmaMac>
  Create(void) const;

private:
  ns3::ObjectFactory m_mac;
};

} //namespace stdma

#endif /* STDMA_MAC_HELPER_H */
