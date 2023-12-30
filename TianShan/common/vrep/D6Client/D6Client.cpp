#include <ZQ_common_conf.h>
#include <VrepSpeaker.h>
#include <FileLog.h>
#include <getopt.h>
#include <strHelper.h>
using namespace ZQ::common;
using namespace ZQ::Vrep;

#define str2bytes(str) bytes(str.begin(), str.end())
#define str2int(str) strtol(str.c_str(), NULL, 10)

class ConsoleLog: public Log {
public:
    ConsoleLog():Log(Log::L_DEBUG) {
    }
    virtual void writeMessage(const char *msg, int level /*=-1*/)
    {
        printf("%s\t%s\n", getVerbosityStr(level), msg);
    }
};
ConsoleLog CONSOLE;

void showUsage() {
    printf(
        "%s\n",
        "usage: d6client [-l LogPath] <ServerIP:Port> <Attributes>\n"
        "       d6client -h\n"
        "   Attributes:\n"
        "       -V vrep parameters: id=Identifier,zone=StreamingZone,name=ComponentName\n"
        "       -W withdraw routes: Address\n"
        "       -R reachable routes: Address\n"
        "       -H next hop server: addr=ComponentAddress,zone=StreamingZone\n"
        "       -N QAM names: Name\n"
        "       -T total bandwidth: Bandwidth\n"
        "       -A available bandwidth: Bandwidth\n"
        "       -P QAM parameter: freq=FrequencyKHz,mod=ModulationMode,intl=Interleaver,tsid=TSID\n"
        "       -U UDP map: port=StartPort,pn=StartPN,count=Count\n"
        );
}
typedef std::map<std::string, std::string> SimpleSetting;
// format of config text:
//      key1=value1,key2=value2
// keys are case-insensitive
static void parseConfig(const char* pCfgText, SimpleSetting &cfg);
static bool getParam(const SimpleSetting&, const char*, std::string&);
int
main(int argc, char* argv[]) {
    if(argc < 2) {
        showUsage();
        return 0;
    }

    std::string svrIp, svrPort, logPath,
        vrepIdentifier, vrepStreamingZone, vrepComponentName,
        withdrawRouteAddr,
        reachableRouteAddr,
        nhsAddr, nhsStreamingZone,
        qamName,
        totalBw,
        availBw,
        qamFreq, qamMod, qamIntl, qamTsid,
        udpPort, udpPn, udpCount
        ;
    SimpleSetting tmpSetting;

    int opt = -1;
    while((opt = getopt(argc, argv, "hl:V:W:R:H:N:T:A:P:U:")) != -1)
    {
        switch(opt)
        {
        case 'h':
            showUsage();
            return 0;
        case 'l':
            logPath = optarg;
            CONSOLE(Log::L_DEBUG, "Get log path: %s", logPath.c_str());
            break;
        case 'V':
            parseConfig(optarg, tmpSetting);
            if(!getParam(tmpSetting, "id", vrepIdentifier)) {
                CONSOLE(Log::L_ERROR, "Parameter missed: VrepParameters::Identifier");
                return -1;
            }
            if(!getParam(tmpSetting, "zone", vrepStreamingZone)) {
                CONSOLE(Log::L_ERROR, "Parameter missed: VrepParameters::StreamingZone");
                return -1;
            }
            if(!getParam(tmpSetting, "name", vrepComponentName)) {
                CONSOLE(Log::L_ERROR, "Parameter missed: VrepParameters::ComponentName");
                return -1;
            }
            tmpSetting.clear();
            CONSOLE(Log::L_DEBUG, "Get vrep parameters: Identifier=%s, StreamingZone=%s, ComponentName=%s", vrepIdentifier.c_str(), vrepStreamingZone.c_str(), vrepComponentName.c_str());
            break;
        case 'W':
            withdrawRouteAddr = optarg;
            CONSOLE(Log::L_DEBUG, "Get withdraw route address: %s", withdrawRouteAddr.c_str());
            break;
        case 'R':
            reachableRouteAddr = optarg;
            CONSOLE(Log::L_DEBUG, "Get reachable route address: %s", reachableRouteAddr.c_str());
            break;
        case 'H':
            parseConfig(optarg, tmpSetting);
            if(!getParam(tmpSetting, "addr", nhsAddr)) {
                CONSOLE(Log::L_ERROR, "Parameter missed: NextHopServer::ComponentAddress");
                return -1;
            }
            if(!getParam(tmpSetting, "zone", nhsStreamingZone)) {
                CONSOLE(Log::L_ERROR, "Parameter missed: NextHopServer::StreamingZone");
                return -1;
            }
            tmpSetting.clear();
            CONSOLE(Log::L_DEBUG, "Get next hop server: ComponentAddress=%s, StreamingZone=%s", nhsAddr.c_str(), nhsStreamingZone.c_str());
            break;
        case 'N':
            qamName = optarg;
            CONSOLE(Log::L_DEBUG, "Get qam name: %s", qamName.c_str());
            break;
        case 'T':
            totalBw = optarg;
            CONSOLE(Log::L_DEBUG, "Get total bandwidth in kbps: %s", totalBw.c_str());
            break;
        case 'A':
            availBw = optarg;
            CONSOLE(Log::L_DEBUG, "Get available bandwidth in kbps: %s", availBw.c_str());
            break;
        case 'P':
            parseConfig(optarg, tmpSetting);
            if(!getParam(tmpSetting, "freq", qamFreq)) {
                CONSOLE(Log::L_ERROR, "Parameter missed: QAMParameter::FrequencyKHz");
                return -1;
            }
            if(!getParam(tmpSetting, "mod", qamMod)) {
                CONSOLE(Log::L_ERROR, "Parameter missed: QAMParameter::ModulationMode");
                return -1;
            }
            if(!getParam(tmpSetting, "intl", qamIntl)) {
                CONSOLE(Log::L_ERROR, "Parameter missed: QAMParameter::Interleaver");
                return -1;
            }
            if(!getParam(tmpSetting, "tsid", qamTsid)) {
                CONSOLE(Log::L_ERROR, "Parameter missed: QAMParameter::TSID");
                return -1;
            }
            tmpSetting.clear();
            CONSOLE(Log::L_DEBUG, "Get qam pamameters: FrequencyKHz=%s, ModulationMod=%s, Interleaver=%s, TSID=%s", qamFreq.c_str(), qamMod.c_str(), qamIntl.c_str(), qamTsid.c_str());
            break;
        case 'U':
            parseConfig(optarg, tmpSetting);
            if(!getParam(tmpSetting, "port", udpPort)) {
                CONSOLE(Log::L_ERROR, "Parameter missed: UDPMap::StartPort");
                return -1;
            }
            if(!getParam(tmpSetting, "pn", udpPn)) {
                CONSOLE(Log::L_ERROR, "Parameter missed: UDPMap::StartPn");
                return -1;
            }
            if(!getParam(tmpSetting, "count", udpCount)) {
                CONSOLE(Log::L_ERROR, "Parameter missed: UDPMap::Count");
                return -1;
            }
            tmpSetting.clear();
            CONSOLE(Log::L_DEBUG, "Get udp map: StartPort=%s, StartPN=%s, Count=%s", udpPort.c_str(), udpPn.c_str(), udpCount.c_str());
            break;
        case '?':
            if(strchr("lVWRHNTAPU", optopt))
            {
                CONSOLE(Log::L_WARNING, "need an argument with -%c", optopt);
            }
            else
            {
                CONSOLE(Log::L_WARNING, "unknown option -%c", optopt);
            }
            return -1;
        }
    }    

    if(optind == argc)
    {
        CONSOLE(Log::L_ERROR, "Need the server endpoint");
        return -1;
    }
    else
    {
        std::string svrEp = argv[optind];
        std::string::size_type pos = svrEp.find(':');
        if(pos != std::string::npos) {
            svrIp = svrEp.substr(0, pos);
            svrPort = svrEp.substr(pos + 1);
            CONSOLE(Log::L_DEBUG, "Get verp server endpoint: %s:%s", svrIp.c_str(), svrPort.c_str());
        } else {
            CONSOLE(Log::L_ERROR, "Bad server endpoint: %s", svrEp.c_str());
            return -1;
        }
    }

    if(vrepIdentifier.empty()) {
        CONSOLE(Log::L_ERROR, "Need vrep parameters");
        return -1;
    }
    Log* pLog = &CONSOLE;
    if(logPath.empty()) {
        CONSOLE(Log::L_INFO, "send output to console");
    } else {
        try {
            pLog = new FileLog(logPath.c_str(), Log::L_DEBUG);
            CONSOLE(Log::L_INFO, "send output to %s", logPath.c_str());
        } catch (const FileLogException& e) {
            CONSOLE(Log::L_WARNING, "Failed to create log file %s due to: %s. Redirect output to console.", logPath.c_str(), e.getString());
            pLog = &CONSOLE;
        }
    }
    NativeThreadPool thrdPool;
    Speaker speaker(*pLog, thrdPool);
    speaker.setPeer(svrIp.c_str(), str2int(svrPort));
    speaker.enableAutoRestart(10);
    ZQ::Vrep::Configuration conf;
    conf.identifier = str2int(vrepIdentifier);
    conf.streamingZone = vrepStreamingZone;
    conf.componentName = vrepComponentName;
    conf.vendorString = "ZQ D6 test client";
    conf.defaultHoldTimeSec = 10;
    conf.connectRetryTimeSec = 60;
    conf.connectTimeoutMsec = 2000;
    conf.keepAliveTimeSec = 10;
    conf.sendReceiveMode = VREP_SendOnlyMode;
    speaker.start(conf);
    CONSOLE(Log::L_INFO, "Start vrep speaker");

    { // get the update message
        UpdateMessage msg;
        // withdraw routes
        if(!withdrawRouteAddr.empty()) {
            Routes rs;
            Route r;
            r.family = 0x8001;
            r.protocol = 0x8000;
            r.address = str2bytes(withdrawRouteAddr);
            rs.push_back(r);
            msg.setWithdrawnRoutes(rs);
        }
        // reachable routes
        if(!reachableRouteAddr.empty()) {
            Routes rs;
            Route r;
            r.family = 0x8001;
            r.protocol = 0x8000;
            r.address = str2bytes(reachableRouteAddr);
            rs.push_back(r);
            msg.setReachableRoutes(rs);
        }
        // next hop server
        if(!nhsAddr.empty()) {
            NextHopServer nhs;
            nhs.componentAddress = str2bytes(nhsAddr);
            nhs.streamingZone = str2bytes(nhsStreamingZone);
            msg.setNextHopServer(nhs);
        }
        // qam names
        if(!qamName.empty()) {
            QAMNames qamNames;
            qamNames.push_back(qamName);
            msg.setQAMNames(qamNames);
        }
        // toatal bandwidth
        if(!totalBw.empty()) {
            msg.setTotalBandwidth(str2int(totalBw));
        }
        // avail bandwidth
        if(!availBw.empty()) {
            msg.setAvailableBandwidth(str2int(availBw));
        }
        // qam parameter
        if(!qamFreq.empty()) {
            QAMParameters qamParam;
            qamParam.frequencyKHz = str2int(qamFreq);
            qamParam.modulationMode = str2int(qamMod);
            qamParam.interleaver = str2int(qamIntl);
            qamParam.tsid = str2int(qamTsid);
            qamParam.annex = 1;
            qamParam.channelWidth = 1;
            msg.setQAMParameters(qamParam);
        }
        // udp map
        if(!udpPort.empty()) {
            UDPMap udpMap;
            PortMapItem item;
            item.port = str2int(udpPort);
            item.pn = str2int(udpPn);
            item.count = str2int(udpCount);
            udpMap.dynamicPorts.push_back(item);
            msg.setUDPMap(udpMap);
        }
#define D6_TimeOut 3000
        speaker.sendUpdate(msg, D6_TimeOut);
        std::string txtMsg;
        msg.textDump(txtMsg);
        CONSOLE(Log::L_INFO, "Send %s", txtMsg.c_str());
    }
    CONSOLE(Log::L_INFO, "Stop vrep speaker in %d seconds", (D6_TimeOut/1000));
    Sleep(D6_TimeOut);
    speaker.stop();
    CONSOLE(Log::L_INFO, "Stop vrep speaker");
    if(pLog != &CONSOLE) {
        delete pLog;
    }
    return 0;
}

// format of config text:
//      key1=value1, key2=value2
// keys are case-insensitive
static void parseConfig(const char* pCfgText, SimpleSetting &cfg)
{
    cfg.clear();
    if(NULL == pCfgText)
        return;

    using namespace ZQ::common;
    stringHelper::STRINGVECTOR vec;
    stringHelper::SplitString(pCfgText, vec, ",;");
    for(size_t i = 0; i < vec.size(); ++i)
    {
        stringHelper::STRINGVECTOR item;
        stringHelper::SplitString(vec[i], item, "=", "= ");
        if(item.size() != 2)
            continue; // discard bad configuration

        std::transform(item[0].begin(), item[0].end(), item[0].begin(), tolower);
        cfg[item[0]] = item[1];
    }
}
static bool getParam(const SimpleSetting& conf, const char* name, std::string& val) {
    SimpleSetting::const_iterator it = conf.find(name);
    if(it != conf.end()) {
        val = it->second;
        return true;
    } else {
        return false;
    }
}