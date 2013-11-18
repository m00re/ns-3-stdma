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

#include "stdma-test-suite.h"
#include "single-node-test.h"
#include "two-nodes-test.h"
#include "slot-manager-test.h"

using namespace ns3;

namespace stdma {

  StdmaSingleNodeTestSuite::StdmaSingleNodeTestSuite ()
    : ns3::TestSuite ("devices-stdma", UNIT)
  {
    AddTestCase (new StdmaTwoNodesTest);
    AddTestCase (new StdmaSingleNodeTest);
    AddTestCase (new StdmaSlotManagerTest);
  }

  StdmaSingleNodeTestSuite g_stdmaSingleNodeTestSuite;

}
