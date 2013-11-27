NS-3 STDMA: Self-organizing TDMA implementation for NS-3
========================================================

This is an implementation of the protocol Self-Organizing TDMA (STDMA) in 
[NS-3](http://www.nsnam.org/) and is envisioned to be used as a replacement
for the CSMA-based MAC protocol used in the 'wifi' module of NS-3. As such, 
the provided 'stdma' module of this repository re-uses the existing 
WifiPhy-based implementation of the official NS-3 distribution.

STDMA is a time-division multiplexing based protocol initially developed for 
and standardized in the Automatic Identification System (AIS) as well as in 
VDL Mode 4 to periodically exchange positioning information between ships 
and aircrafts.

In contrast to CSMA, which employs a random access based scheme to the wireless 
channel, STDMA implements a reservation based scheme: time is divided in so 
called frames that last for a certain duration, and those frames are further
ivided into equally sized transmission slots that accommodate a single packet 
transmission. The protocol is therefore well suited to be used in active safety 
systems such as Intelligent Transportation Systems (ITS) in general or VANETs 
in particular.

1) Implementation Overview
--------------------------

The current implementation was developed against version 3.18 of NS-3 and has 
not been tested for more recent and neither for earlier versions of NS-3.

Switching from the CSMA-based implementation of the NS-3 wifi module to the 
above STDMA implementation is quite simple and requires only a few line changes 
in your simulation script (see section 2 below). 

What should be kept in mind when using the STDMA implementation:

 - Nodes/applications need to generate packets on a periodic basis and it is 
   necessary that there is always one packet ready to be transmitted if a 
   reserved transmission slot is awaiting.
 - The minimum transmission rate supported is 2 packets per frame
 - Variable transmission rates are not supported

2) How to use the STDMA implementation
--------------------------------------

If you have the following lines of code to configure your Wifi nodes

```C++
#include "ns3/wifi-module.h"

using namespace ns3;

// Create 10 nodes
NodeContainer m_nodes;
m_nodes.Create(10);

// Create Wifi MAC helpers...
WifiHelper wifiHelper = WifiHelper::Default ();
NqosWifiMacHelper wifiMac = NqosWifiMacHelper::Default ();
// ... and configure them as desired
wifiHelper.SetStandard(WIFI_PHY_STANDARD_80211p_CCH);
wifiMac.SetType ("ns3::AdhocWifiMac");
wifiHelper.SetRemoteStationManager ("ns3::ConstantRateWifiManager",
                                    "DataMode", StringValue (datarate),
                                    "NonUnicastMode", StringValue (datarate));

// Create the wireless channel configuration
YansWifiChannelHelper wifiChannel;
wifiChannel.AddPropagationLoss("ns3::RangePropagationLossModel");
wifiChannel.SetPropagationDelay("ns3::ConstantSpeedPropagationDelayModel");
YansWifiPhyHelper wifiPhy = YansWifiPhyHelper::Default();
wifiPhy.SetChannel(wifiChannel.Create());

// And finally install/apply the Wifi configuration on all nodes
wifiHelper.Install(wifiPhy, wifiMac, m_nodes);
```

you simply need to change the above code block to look like the one below

```C++
#include "ns3/wifi-module.h"
#include "ns3/stdma-module.h"

using namespace ns3;

// Create 10 nodes
NodeContainer m_nodes;
m_nodes.Create(10);

// Create Wifi MAC helpers...
stdma::StdmaHelper stdmaHelper;
stdma::StdmaMacHelper stdmaMac = stdma::StdmaMacHelper::Default();
// ... and configure them as desired
stdmaHelper.SetStandard(WIFI_PHY_STANDARD_80211p_CCH);

// Create the wireless channel configuration
YansWifiChannelHelper wifiChannel;
wifiChannel.AddPropagationLoss("ns3::RangePropagationLossModel");
wifiChannel.SetPropagationDelay("ns3::ConstantSpeedPropagationDelayModel");
YansWifiPhyHelper wifiPhy = YansWifiPhyHelper::Default();
wifiPhy.SetChannel(wifiChannel.Create());

// And finally install/apply the Wifi configuration on all nodes
stdmaHelper.Install(wifiPhy, stdmaMac, m_nodes, startupTimes);
```

and you are ready to go. 

3) Complete example scenario
--------------------------------------

The module ships with a simple but fully functional example scenario that is built
as long as NS-3 is built with examples enabled. The source code to this example is
located in src/stdma/examples/simple-stdma-example.cc and consists of a scenario
in which 10 nodes startup simultaneously and then transmit 10 packets/sec over a 
time duration of 20 seconds.
