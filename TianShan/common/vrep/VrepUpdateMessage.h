#ifndef __ZQ_Vrep_UpdateMessage_H__
#define __ZQ_Vrep_UpdateMessage_H__
#include "VrepMessage.h"
#include <bitset>

namespace ZQ {
namespace Vrep {

// vrep attribute type code
#define VREP_Type_WithdrawnRoutes 1
#define VREP_Type_ReachableRoutes 2
#define VREP_Type_NextHopServer 3
#define VREP_Type_QAMNames 232
#define VREP_Type_CASCapability 233
#define VREP_Type_TotalBandwidth 234
#define VREP_Type_AvailableBandwidth 235
#define VREP_Type_Cost 236
#define VREP_Type_EdgeInput 237
#define VREP_Type_QAMParameters 238
#define VREP_Type_UDPMap 239
#define VREP_Type_Volume 240
#define VREP_Type_ServiceStatus 241
#define VREP_Type_MaxMPEGFlows 242
#define VREP_Type_NextHopServerAlternates 243
#define VREP_Type_OutputPort 244
#define VREP_Type_OutputAddress 245
#define VREP_Type_TransferProtocolCapabilities 246

// transfer protocol capabilities bitmask
#define VREP_TransferProtocolCap_FTP 0x01
#define VREP_TransferProtocolCap_NFS 0x02
#define VREP_TransferProtocolCap_CIFS 0x04
#define VREP_TransferProtocolCap_PGM 0x08

/* Route
  0                   1                   2                   3
  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
  +---------------+---------------+--------------+----------------+
  |       Address Family          |      Application Protocol     |
  +---------------+---------------+--------------+----------------+
  |            Length(address)    |       Address (variable)     ...
  +---------------+---------------+--------------+----------------+
*/
/* Route Name
  +---------------+---------------+--------------+----------------+
  |            Length(name)       |     RouteName (variable)     ...
  +---------------+---------------+--------------+----------------+

*/
struct Route {
    word family;
    word protocol;
    bytes address;
    bytes name;
};
typedef std::vector<Route> Routes;
/*
  0                   1                   2                   3
  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
  +---------------+---------------+--------------+----------------+
  |                            RESERVED                           |
  +---------------+---------------+--------------+----------------+
  |   Component addr Length       | Component addr (var)  ...
  +---------------+---------------+--------------+----------------+
  |   Streaming Zone Name Length  | Streaming Zone Name (var)бн
  +---------------+---------------+--------------+----------------+
*/
struct NextHopServer {
    bytes componentAddress;
    bytes streamingZone;
};
/*
  0                   1                   2                   3
  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
  +---------------+---------------+--------------+----------------+
  |             NumAlternates     |         Length 1              |
  +---------------+---------------+--------------+----------------+
  |                   Server 1 (variable)    ...                   
  +---------------+---------------+--------------+----------------+
  |             Length N          |          Server N (variable) ...     
  +---------------+---------------+--------------+----------------+
*/
typedef std::vector<std::string> ServerAlternates;

/*
0                   1                   2                   3
0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
+---------------+---------------+--------------+----------------+
| QAM Name 1 Length             | QAM Name 1 (var)бн
+---------------+---------------+--------------+----------------+
|                             бн
+---------------+---------------+--------------+----------------+
|        QAM Name N Length      |  QAM Name N (var)...
+---------------+---------------+--------------+----------------+
*/
typedef std::vector<std::string> QAMNames;
/*
0                   1                   2                   3
0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
+---------------+---------------+--------------+----------------+
|                        Frequency (in KHz)                     |
+---------------+---------------+--------------+----------------+
+ Mod mode      | Interleaver   |       TSID                    |
+---------------+---------------+--------------+----------------+
+ Annex         | Chan width    |   Reserved                    |
+---------------+---------------+--------------+----------------+
*/
struct QAMParameters {
    dword frequencyKHz;
    byte modulationMode;
    byte interleaver;
    word tsid;
    byte annex;
    byte channelWidth;
};

/*
0                   1                   2                   3
0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
+---------------+---------------+--------------+----------------+
|                        Num Static Ports                       |
+---------------+---------------+--------------+----------------+
+             UDP Port 1        |          MPEG Program         |
+---------------+---------------+--------------+----------------+
|                            бн                                  |
+---------------+---------------+--------------+----------------+
|             UDP Port N        |          MPEG Program         | 
+---------------+---------------+--------------+----------------+
+             Num Dynamic Port Ranges                           |
+---------------+---------------+--------------+----------------+
+  starting port 1              | starting MPEG Program 1       |
+---------------+---------------+--------------+----------------+
+                              Count 1                          |
+---------------+---------------+--------------+----------------+
|                            бн                                  |
+---------------+---------------+--------------+----------------+
+  starting port 1              | starting MPEG Program 1       |
+---------------+---------------+--------------+----------------+
+                              Count n                          |
+---------------+---------------+--------------+----------------+
*/
struct PortMapItem {
    word port; // start udp port
    word pn; // start program number
    dword count; // range of the map, step is always 1.
};
typedef std::vector<PortMapItem> PortMap;
struct UDPMap {
    PortMap staticPorts; // one port pair represent one port-pn map
    PortMap dynamicPorts; // one pair represent the range of 
};

/*
  0                   1                   2                   3
  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
  +---------------+---------------+--------------+----------------+
  |      volume 1 Name Length     |         Volume1 (variable)  ...
  +---------------+---------------+--------------+----------------+
  |                             Port ID                           |
  +---------------+---------------+--------------+----------------+
  |              Available Read Bandwidth (Kbps)                  |
  +---------------+---------------+--------------+----------------+
  |              Available Write Bandwidth (Kbps)                 |
  +---------------+---------------+--------------+----------------+
  |             volume Length     |         VolumeN (variable)    ...
  +---------------+---------------+--------------+----------------+
  |                             Port ID                           |
  +---------------+---------------+--------------+----------------+
  |              Available Read Bandwidth (Kbps)                  |
  +---------------+---------------+--------------+----------------+
  |              Available Write Bandwidth (Kbps)                 |
  +---------------+---------------+--------------+----------------+
*/
#define VREP_MsgSize_Volume_Min 14 // 0-length volume name
struct Volume {
    bytes name;
    dword portId;
    dword readBw;
    dword writeBw;
};
typedef std::vector<Volume> Volumes;

/* Service Status
  0                   1                   2                   3
  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
  +---------------+---------------+--------------+----------------+
  |                         Service Status                        |
  +---------------+---------------+--------------+----------------+
*/
#define VREP_ServiceStatus_Invalid 0
#define VREP_ServiceStatus_Operational 1
#define VREP_ServiceStatus_ShuttingDown 2
#define VREP_ServiceStatus_Standby 3

/* Output Address
   0                   1                   2                   3
   0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
   +---------------+---------------+--------------+----------------+
   |        Address length         |  Address (variable)
   +---------------+---------------+--------------+----------------+
*/
/* Max MPEG Flows
  0                   1                   2                   3
  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
  +---------------+---------------+--------------+----------------+
  |                         Max MPEG Flows                        |
  +---------------+---------------+--------------+----------------+
*/
/* Output port
  0                   1                   2                   3
  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
  +---------------+---------------+--------------+----------------+
  |                         Port ID                               |
  +---------------+---------------+--------------+----------------+
*/

/* Attribute
  0                   1                   2                   3
  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
  +---------------+---------------+--------------+----------------+
  |  Attr. Flags  |Attr. Type Code|         Attr. Length          |
  +---------------+---------------+--------------+----------------+
  |                   Attribute Value (variable)                  |
  +---------------+---------------+--------------+----------------+
*/

struct GenericAttribute {
    byte flags;
    byte type;
    bytes data;
};
typedef std::vector<GenericAttribute> GenericAttributes;

/*
0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
+---------------+---------------+--------------+----------------+
|                      Input 1 Subnet Mask                      |
+---------------+---------------+--------------+----------------+
|            Length             |     Input 1 host (variable) ...
+---------------+---------------+--------------+----------------+
|                         Input 1 Port ID                       |
+---------------+---------------+--------------+----------------+
|                   Input 1 Max Group Bandwidth                 |
+---------------+---------------+--------------+----------------+
|            Length             | Input 1 Group Name (variable) ...
+---------------+---------------+--------------+----------------+
|                      Input n Subnet Mask                      |
+---------------+---------------+--------------+----------------+
|            Length             | Input n host (variable) ...
+---------------+---------------+--------------+----------------+
|                        Input n Port ID                        |
+---------------+---------------+--------------+----------------+
|                   Input n Max Group Bandwidth                 |
+---------------+---------------+--------------+----------------+
|             Length            | Input n Group Name (variable) ...
+---------------+---------------+--------------+----------------+
*/
struct EdgeInput{
	dword		subnetMask;
	bytes		host;
	dword		portId;
	dword		maxBW;
	bytes		groupName;
};
typedef std::vector<EdgeInput> EdgeInputs;

class UpdateAttributes
{
public:
    UpdateAttributes();
    ~UpdateAttributes();
    void setWithdrawnRoutes(const Routes& routes);
    bool getWithdrawnRoutes(Routes& routes) const;

    void setReachableRoutes(const Routes& routes);
    bool getReachableRoutes(Routes& routes) const;

    void setNextHopServer(const NextHopServer& srv);
    bool getNextHopServer(NextHopServer& srv) const;

    void setQAMNames(const QAMNames& qamNames);
    bool getQAMNames(QAMNames& qamNames) const;

    void setTotalBandwidth(dword bw);
    bool getTotalBandwidth(dword& bw) const;

    void setAvailableBandwidth(dword bw);
    bool getAvailableBandwidth(dword& bw) const;

    void setCost(byte cost);
    bool getCost(byte& cost) const;

    void setQAMParameters(const QAMParameters& qamParams);
    bool getQAMParameters(QAMParameters& qamParams) const;

    void setUDPMap(const UDPMap& udpMap);
    bool getUDPMap(UDPMap& udpMap) const;

    void setVolumes(const Volumes& vols);
    bool getVolumes(Volumes& vols);

    void setServiceStatus(dword st);
    bool getServiceStatus(dword& st) const;

    void setMaxMPEGFlows(dword nFlows);
    bool getMaxMPEGFlows(dword& nFlows) const;

    void setNextHopServerAlternates(const ServerAlternates& alts);
    bool getNextHopServerAlternates(ServerAlternates& alts) const;

    void setOutputPort(dword port);
    bool getOutputPort(dword& port) const;

    void setOutputAddress(const bytes& addr);
    bool getOutputAddress(bytes& addr) const;

    void setTransferProtocolCapabilities(byte cap);
    bool getTransferProtocolCapabilities(byte& cap) const;

    void setGenericAttributes(const GenericAttributes& attrs);
    bool getGenericAttributes(GenericAttributes& attrs) const;

    void setEdgeInputs(const EdgeInputs& edgeInputs);
    bool getEdgeInputs(EdgeInputs& edgeInputs)const;
public:
    bool validate() const;
    // parse&build
    int parse(const byte* buf, size_t len);
    int build(byte* buf, size_t len) const;
    void clear();

    void textDump(std::string& data) const;
private:
    Routes withdrawnRoutes_;
    Routes reachableRoutes_;
    NextHopServer nextHopServer_;
    QAMNames qamNames_;
    EdgeInputs edgeInputs_;
    // 6.2.4.4	CAS Capability
    dword totalBandwidth_;
    dword availableBandwidth_;
    byte cost_;
    // 6.2.5	Edge Input
    QAMParameters qamParameters_;
    UDPMap udpMap_;
    Volumes volumes_;
    dword serviceStatus_;
    dword maxMPEGFlows_;
    ServerAlternates nhsAlternates_;
    dword outputPort_;
    bytes outputAddress_;
    byte transferProtocolCap_;

    GenericAttributes genericAttrs_;
#define Attr_Count 20
    std::bitset<Attr_Count> setting_;
};
typedef UpdateAttributes UpdateMessage;
}} // namespace ZQ::Vrep
#endif
