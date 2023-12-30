#include <TianShanDefines.h>
#include <../CDNDefines.h>
#include <C2Locator.h>
#include <TianShanIceHelper.h>
#include <Log.h>
#include <Text.h>
#include <getopt.h>

using namespace ZQ::common;
using namespace TianShanIce::SCS;

class ConsoleLog: public ZQ::common::Log
{
public:
    ConsoleLog();
	virtual void writeMessage(const char *msg, int level=-1);
};

ConsoleLog gLog;

static void printClients(const ClientTransfers& clients, int countOnly)
{
    printf("==========================\n");
    printf("@total %d clients\n", clients.size());
    if(!countOnly)
    {
        printf("--------------------------\n");
        for(size_t i = 0; i < clients.size(); ++i)
        {
            printf("%d:\n", i + 1);
            printf("\taddress:\t%s\n", clients[i].address.c_str());
            printf("\tsessions:\t%lld\n", clients[i].activeTransferCount);
            printf("\tconsumed:\t%lld\n", clients[i].consumedBandwidth);
            printf("\tcapacity:\t%lld\n", clients[i].ingressCapacity);
            printf("\n");
        }
    }
    printf("==========================\n");
}

static void printPorts(const TransferPorts& ports, int countOnly)
{
    printf("==========================\n");
    printf("@total %d ports\n", ports.size());
    if(!countOnly)
    {
        printf("--------------------------\n");
        for(size_t i = 0; i < ports.size(); ++i)
        {
            printf("%d:\n", i + 1);
            printf("\tname:\t\t%s\n", ports[i].name.c_str());
            printf("\tstatus:\t\t%s\n", ports[i].isUp ? "UP" : "DOWN");
            printf("\tsessions:\t%lld\n", ports[i].activeTransferCount);
            printf("\tconsumed:\t%lld\n", ports[i].activeBandwidth);
            printf("\tcapacity:\t%lld\n", ports[i].capacity);
            printf("\tStreamService:\t%s\n", ports[i].streamService.c_str());
            printf("\n");
        }
    }
    printf("==========================\n");
}

static void printSessions(const TransferSessions& sess, int countOnly)
{
    printf("==========================\n");
    printf("@total %d sessions\n", sess.size());
    if(!countOnly)
    {
        printf("--------------------------\n");
        for(size_t i = 0; i < sess.size(); ++i)
        {
            printf("%d:\n", i + 1);
            printf("\tid:\t%s\n", sess[i].transferId.c_str());
            printf("\tclient:\t%s\n", sess[i].clientTransfer.c_str());
            printf("\tport:\t%s\n", sess[i].transferPort.c_str());
            printf("\tallocatedBw:\t%lld\n", sess[i].allocatedBW);
            std::string val;
            ZQTianShan::Util::getValueMapDataWithDefault(sess[i].others, CDN_PID, "", val);
            printf("\tpid:\t%s\n", val.c_str());
            ZQTianShan::Util::getValueMapDataWithDefault(sess[i].others, CDN_PAID, "", val);
            printf("\tpaid:\t%s\n", val.c_str());
            ZQTianShan::Util::getValueMapDataWithDefault(sess[i].others, CDN_SUBTYPE, "", val);
            if(val.empty())
            {
                ZQTianShan::Util::getValueMapDataWithDefault(sess[i].others, CDN_EXTENSIONNAME, "", val);
            }
            printf("\tsubtype/ext:%s\t\n", val.c_str());

            printf("\n");
        }
    }
    printf("==========================\n");
}

void showHelp()
{
    printf("\nUsage: c2loc HOST [-h] [-l c|p|s|sc|sp] [-x 0|1] [-q queries] [-P port] [-C]\n");
    printf("\nOptions:\n"
           "    -h Print this message.\n"
           "    -l List clients, ports or sessions. valid option can be:\n"
           "        c  list clients info.\n"
           "        p  list ports info.\n"
           "        s  list sessions info.\n"
           "        sc list sessions info by clients. Must provide the client list  in -q\n"
           "        sp list sessions info by ports. Must provide the ports list  in -q\n"
           "    -x 1 for enable, 0 for disable the ports. Must provide the port list in -q\n"
           "    -q Provide the query string.\n"
           "    -P The tcp port of the locate service.\n"
           "    -C Only print the result's count only.\n");
}
int
main(int argc, char* argv[])
{
    if(1 == argc)
    {
        showHelp();
        return -1;
    }
    const char* host = NULL, *tcpPort = NULL;
    int countOnly = 0;
    enum{
        eNone, eClients,
        ePorts, eSessions,
        eSessionsC, // sessions by clients
        eSessionsP  // sessions by ports
    } listMethod = eNone;

    int enablePorts = -1;

    const char* queries = NULL;
    int opt = -1;
    while((opt = getopt(argc, argv, "hCl:x:q:P:")) != -1)
    {
        switch(opt)
        {
        case 'h':
            showHelp();
            return 0;
        case 'C':
            countOnly = 1;
            break;
        case 'l':
            if(0 == stricmp("c", optarg))
                listMethod = eClients;
            else if(0 == stricmp("p", optarg))
                listMethod = ePorts;
            else if(0 == stricmp("s", optarg))
                listMethod = eSessions;
            else if(0 == stricmp("sc", optarg))
                listMethod = eSessionsC;
            else if(0 == stricmp("sp", optarg))
                listMethod = eSessionsP;
            else
            {
                printf("Unknown list method: %s\n", optarg);
                return -1;
            }
            break;
        case 'x':
            enablePorts = atoi(optarg);
            break;
        case 'q':
            queries = optarg;
            break;
        case 'P':
            tcpPort = optarg;
            break;
        case '?':
            if(strchr("lxqP", optopt))
            {
                printf("need an argument with -%c\n", optopt);
            }
            else
            {
                printf("unknown option -%c\n", optopt);
            }
            return -1;
        }
    }    

    if(optind == argc)
    {
        printf("Need the host name or address of the target locator\n");
        return -1;
    }
    else
    {
        host = argv[optind];
    }
    // check the option
    if(NULL == tcpPort || atoi(tcpPort) <= 0)
        tcpPort = "6789";

    if(listMethod == eNone)
    {
        if(enablePorts != -1)
        {
            if(NULL == queries)
            {
                printf("Require -q with -x\n");
                return -1;
            }
        }
        else
        { // default list ports
            listMethod = ePorts;
        }
    }
    else
    {
        if(enablePorts != -1)
        {
            printf("Option -l conflict with -x.\n");
            return -1;
        }
        else
        {
            if(listMethod == eSessionsC && queries == NULL)
            {
                printf("Require -q with -l sc\n");
                return -1;
            }
            if(listMethod == eSessionsP && queries == NULL)
            {
                printf("Require -q with -l sp\n");
                return -1;
            }
        }
    }

    std::string locatorProxyStr = std::string("C2Locator: tcp -h ") + host + " -p " + tcpPort;
    Ice::InitializationData initData;
    Ice::CommunicatorPtr comm = Ice::initialize(initData);
    try
    {
        printf("\n*connecting locator: %s ...", locatorProxyStr.c_str());
        C2LocatorPrx loc = C2LocatorPrx::checkedCast(comm->stringToProxy(locatorProxyStr));

        printf("done*\n");

        if(listMethod != eNone)
        {
            switch(listMethod)
            {
            case eClients:
                {
                    printf("\n*listing the clients...");
                    ClientTransfers clients = loc->listClients();
                    printf("done*\n");
                    printClients(clients, countOnly);
                }
                break;
            case ePorts:
                {
                    printf("\n*listing the ports...");
                    TransferPorts ports = loc->listTransferPorts();
                    printf("done*\n");
                    printPorts(ports, countOnly);
                }
                break;
            case eSessions:
                {
                    TransferSessions sess;
                    printf("*enumerating the clients...");
                    ClientTransfers clients = loc->listClients();
                    printf("done\n");
                    std::vector<std::string> vals;
                    for(ClientTransfers::const_iterator it = clients.begin(); it != clients.end(); ++it)
                        vals.push_back(it->address);

                    printf("*listing the sessions info of clients %s...", ZQ::common::Text::join(vals).c_str());
                    for(size_t i = 0; i < vals.size(); ++i)
                    {
                        TransferSessions ses = loc->listSessionsByClient(vals[i]);
                        sess.insert(sess.end(), ses.begin(), ses.end());
                    }
                    printf("done\n");
                    printSessions(sess, countOnly);
                }
                break;
            case eSessionsC:
                {
                    TransferSessions sess;
                    std::vector<std::string> vals;
                    ZQ::common::Text::split(vals, queries, ", ");
                    printf("*listing the sessions info of clients %s...", ZQ::common::Text::join(vals).c_str());
                    for(size_t i = 0; i < vals.size(); ++i)
                    {
                        TransferSessions ses = loc->listSessionsByClient(vals[i]);
                        sess.insert(sess.end(), ses.begin(), ses.end());
                    }
                    printf("done\n");
                    printSessions(sess, countOnly);
                }
                break;
            case eSessionsP:
                {
                    TransferSessions sess;
                    std::vector<std::string> vals;
                    ZQ::common::Text::split(vals, queries, ", ");
                    printf("*listing the sessions info of ports %s...", ZQ::common::Text::join(vals).c_str());
                    for(size_t i = 0; i < vals.size(); ++i)
                    {
                        TransferSessions ses = loc->listSessionsByPort(vals[i]);
                        sess.insert(sess.end(), ses.begin(), ses.end());
                    }
                    printf("done\n");
                    printSessions(sess, countOnly);
                }
                break;
            };
        }
        else
        {
            TianShanIce::StrValues vals;
            printf("*%s ports %s...", (enablePorts ? "Enable" : "Disable"), queries);
            loc->updatePortsAvailability(ZQ::common::Text::split(vals, queries, ", "), (enablePorts ? true : false));
            printf("done\n");
        }
    }
    catch(const Ice::Exception& e)
    {
        gLog(Log::L_ERROR, "%s caught. desc:%s", e.ice_name().c_str(), e.what());
    }
    gLog.flush();
    printf("\nEND\n");
    return 0;
}

ConsoleLog::ConsoleLog()
{
    setVerbosity(ZQ::common::Log::L_DEBUG);
}
void ConsoleLog::writeMessage(const char *msg, int level)
{
    const char* lvl = "";
    switch(level)
    {
    case ZQ::common::Log::L_DEBUG:
        lvl = "DEBUG";
        break;

    case ZQ::common::Log::L_INFO:
        lvl = "INFO";
        break;
    case ZQ::common::Log::L_WARNING:
        lvl = "WARNING";
        break;
    case ZQ::common::Log::L_ERROR:
        lvl = "ERROR";
        break;
    case ZQ::common::Log::L_EMERG:
        lvl = "EMERG";
        break;
    default:
        lvl = "UNKNOWN";
        break;
    }
    printf("\n[%s]\t%s\n", lvl, msg);
}
