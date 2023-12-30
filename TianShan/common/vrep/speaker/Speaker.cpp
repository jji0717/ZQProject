#include "VrepSpeaker.h"
#include <FileLog.h>

using namespace ZQ::common;
using namespace ZQ::Vrep;
static bytes stringToBytes(const std::string& str) {
    return bytes(str.begin(), str.end());
}
int
main(int, char**) {
    FileLog* pLog = new FileLog("C:\\VrepSpeaker.log", Log::L_DEBUG);
    NativeThreadPool thrdPool;
    Speaker speaker(*pLog, thrdPool);
    speaker.setPeer("10.15.10.21", 7051);
    speaker.enableAutoRestart(10);
    ZQ::Vrep::Configuration conf;
    conf.identifier = ntohl(inet_addr("192.168.81.103"));
    conf.streamingZone = "ZQ";
    conf.componentName = "Vrep";
    conf.vendorString = "Speaker";
    conf.defaultHoldTimeSec = 240;
    conf.connectRetryTimeSec = 60;
    conf.connectTimeoutMsec = 2000;
    conf.keepAliveTimeSec = 60;
    conf.sendReceiveMode = VREP_SendOnlyMode;
    speaker.start(conf);
    Sleep(1000);
    for(int i = 0; true; ++i) {
    printf("%d\n", i);
    Sleep(100);
    // case 1: empty UPDATE
    {
        UpdateMessage msg;
        speaker.sendUpdate(msg, 1000);
    }
    // case 2: basic reachable route
    {
        UpdateMessage msg;
        // reachable route
        Route r;
        r.family = VREP_AddressFamily_NGOD;
        r.protocol = VREP_AppProtocol_R2;
        r.address = stringToBytes("192.168.81.103");
        r.name = stringToBytes("zq");
        Routes rs;
        rs.push_back(r);

        msg.setReachableRoutes(rs);

        NextHopServer nhs;
        nhs.componentAddress = stringToBytes("192.168.81.103");
        nhs.streamingZone = stringToBytes("ZQ");

        msg.setNextHopServer(nhs); // next hop server
        msg.setTotalBandwidth(0x101010); // total bw
        msg.setAvailableBandwidth(0x101010); // available bw
        
        msg.setOutputPort(0x202);
        std::string addr = "192.168.81.103";
        msg.setOutputAddress(bytes(addr.begin(), addr.end()));


        speaker.sendUpdate(msg, 2000);
    }
    // case 3: Cost
    {
        UpdateMessage msg;
        msg.setCost(0x0F);
        speaker.sendUpdate(msg, 1000);
    }
    // case 4: Volumes
    {
        UpdateMessage msg;
        Volume vol;
        vol.name = stringToBytes("$");
        vol.portId = 0x00ff00ff;
        vol.readBw = 0x00010000;
        vol.writeBw = 0x00010000;

        Volumes vols;
        vols.push_back(vol);
        msg.setVolumes(vols);

        speaker.sendUpdate(msg, 1000);
    }
    // case 5: ServiceStatus
    {
        UpdateMessage msg;
        msg.setServiceStatus(VREP_ServiceStatus_Operational);
        speaker.sendUpdate(msg, 1000);
    }
    // case 6: MaxMPEGFlows
    {
        UpdateMessage msg;
        msg.setMaxMPEGFlows(0x10);
        speaker.sendUpdate(msg, 1000);
    }
    // case 7: Transfer Protocol Capabilities
    {
        UpdateMessage msg;
        msg.setTransferProtocolCapabilities(VREP_TransferProtocolCap_FTP | VREP_TransferProtocolCap_NFS);
        speaker.sendUpdate(msg, 1000);
    }

    // case 8: NextHopServerAlternates
    {
        UpdateMessage msg;
        ServerAlternates alts;
        alts.push_back("192.138.81.119");

        msg.setNextHopServerAlternates(alts);
        speaker.sendUpdate(msg, 0);
    }
    }
    Sleep(10000000);
    return 0;
}
