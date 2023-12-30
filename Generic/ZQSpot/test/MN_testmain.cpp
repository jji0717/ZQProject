// #include "../MPFNode.h"
// #include "../ZQSpotAgent.h"
// #include "../NodeMgr.h"
#include "../ZQSpot.h"
#include "../SpotData.h"

#include "Test.h"
#include "TestI.h"

#include "Guid.h"
#include "InetAddr.h"
#include "log.h"
#include <Ice/Ice.h>
#include <iostream>

#define REG_KEY_MPFNODE			"Software\\SeaChange\\MPFnode"
#define REG_VAR_MPFNODE_ID		"nodeId"
#define REG_VAR_MPFNODE_GROUP	"groupAddr"
#define REG_VAR_MPFNODE_BIND	"bindAddr"
#define DEFAULT_MPFNODE_GROUP	"233.241.221.217"
#define PORT_NODEHEARTBEAT      10005

//////////////////////////////////////////////////////////////////////////

using namespace Test;
using namespace ZQ::common;
using namespace ZQ::Spot;

//////////////////////////////////////////////////////////////////////////

BOOL WINAPI ConsoleHandler(DWORD event);

//////////////////////////////////////////////////////////////////////////
bool bQuit = false;
ZQ::common::Log		gLog;
extern ZQ::common::Log* ZQ::common::pGlog = &gLog;

// ZQ::common::NativeThreadPool		mainThreadPool(20);
//////////////////////////////////////////////////////////////////////////

class TestServer : public Ice::Application
{
public:
    
    TestServer(const SpotEnv::InitParams&	spotInit): _spotInit(spotInit)
    {

    }

    virtual int run(int argc, char* argv[]);

protected:
	const SpotEnv::InitParams&		_spotInit;
};

class TestThread : public ZQ::common::NativeThread {
public:
	TestThread(SpotEnv& spotEnv):
		_spotEnv(spotEnv)
	{

	}

	virtual int run()
	{
		while (true) {
			try {
				TestIfPrx test = TestIfPrx::checkedCast(
					_spotEnv.openInterface("TestIf"));
				if (test) {
					char host[128];
					gethostname(host, sizeof(host));
					char buf[256];
					sprintf(buf, "======= Host: %s, ProcessId: %d =======", 
						host, GetCurrentProcessId());
					test->TestFn(buf);

					printf("............................\n");
					SpotStatusQuery* spotData = _spotEnv.getStatusQuery();
					Variant var;
					if (spotData->getNodeInfo(var, NULL)) {
						for (int i = 0; i < var.size(); i ++) {
							Variant& node = var[i];
							const char* keyName;
							printf("Node:\n");
							for (i = 0; i < node.size(); i ++) {
								keyName = node.key(i).c_str();
								printf("%s = %s\n", keyName, 
									((tstring )node[keyName]).c_str());
							}

						}
					}
					test = NULL;
				}
			} catch (const Ice::Exception & e) {
				std::cerr << e << std::endl;
			}

			Sleep(3000);
		}
		return 0;
	}

protected:
	SpotEnv&		_spotEnv;
};

int
TestServer::run(int argc, char* argv[])
{
	SpotEnv spotEnv(_spotInit);
	// spotEnv.setCurrentMachineDesc();
	if (!spotEnv.initialize(argc, argv))
		return -1;
	Ice::CommunicatorPtr ic = spotEnv.getCommunicator();
	TestIfPtr test1 = new TestIfI;
	spotEnv.exportInterface(test1, "TestIf");
	shutdownOnInterrupt();
	TestThread testThread(spotEnv);
	testThread.start();
	ic->waitForShutdown();
	spotEnv.shutdown();
	ic->destroy();

	return 0;
}

#include "variant.h"

using namespace ZQ::common;

int main(int argc, char* argv[])
{
	if (argc != 3) {
		printf("invalid command line.\n");
		return -1;
	}
	
	/*
	Variant node, spots, spot;
	node.set("NodeId", "{00000107-0000-0010-8000-00AA006D2EA1}");
	node.set("OS", "Windows");
	node.set("CPU", "Intel P4");
	spots.setSize(2);
	spot.set("Application", "Test1");
	spot.set("EndPoint", "localhost:20001");
	spots.set(0, spot);
	spot.set("Application", "Test2");
	spot.set("EndPoint", "localhost:20002");
	spots.set(1, spot);
	node.set("Subdir", spots);
	
	//////////////////////////////////////////////////////////////////////////
	
	// Variant& node = *var;
	int i, j;
	const char* keyName;
	printf("Node:\n");
	for (i = 0; i < node.size() - 1; i ++) {
		keyName = node.key(i).c_str();
		printf("%s = %s\n", keyName, ((tstring )node[keyName]).c_str());
	}

	Variant& spotsVar = node["Subdir"];
	for (i = 0; i < spotsVar.size(); i ++) {
		printf("\tSpot%d:\n", i);
		Variant& spotV = spots[i];
		for (j = 0; j < spotV.size(); j ++) {
			keyName = spotV.key(j).c_str();
			printf("\t%s = %s\n", keyName, ((tstring )spotV[keyName]).c_str());
		
		}
	}

	return 0;
	*/
// read registery for common settings
	ZQ::common::Guid nodeid;

	//ZQ::common::InetMcastAddress	addrGroup;
	//ZQ::common::InetHostAddress		addrCtrlBind;
	std::string multicastGroupAddr = DEFAULT_MPFNODE_GROUP;
	std::string muticastBindAddr;

	srand(time(NULL));
#ifdef _WIN32
	HKEY hk;
	bool bKeyOpened	= false;
	char  buf[40]	= {0};
	DWORD	nLen	= 40;
	DWORD  nType  =  REG_SZ;
	
	if (ERROR_SUCCESS == RegCreateKeyA(HKEY_LOCAL_MACHINE, REG_KEY_MPFNODE, &hk))
	{
		bKeyOpened = true;
		
		if (ERROR_SUCCESS == RegQueryValueExA(hk, REG_VAR_MPFNODE_ID, NULL, &nType, (unsigned char*)buf, &nLen))
			nodeid = buf;
		
		if (ERROR_SUCCESS == RegQueryValueExA(hk, REG_VAR_MPFNODE_GROUP, NULL, &nType, (unsigned char*)buf, &nLen))
		{
			multicastGroupAddr = buf;
		}
		
		if (ERROR_SUCCESS == RegQueryValueExA(hk, REG_VAR_MPFNODE_BIND, NULL, &nType, (unsigned char*)buf, &nLen))
			muticastBindAddr = buf;
	}
	
	if (nodeid.isNil()) {
		nodeid.create();
	}

	nodeid.toString(buf, sizeof(buf));

	if (bKeyOpened)
	{
		RegSetValueExA(hk, REG_VAR_MPFNODE_ID, NULL, REG_SZ, (unsigned char*)buf, strlen(buf)+1);
		RegCloseKey(hk);
	}
#else
#error not implemented
#endif // _WIN32

	SpotEnv::InitParams init;
	init.appName = "MN_Test";
	init.silent = false;
	init.iceServiceAddr = argv[1];
	init.iceServicePort = atoi(argv[2]);	
	init.multicastGroupAddr = multicastGroupAddr;
	init.multicastPort = PORT_NODEHEARTBEAT;
	init.nodeId = buf;

	TestServer server(init);
	server.main(argc, argv);	
	
	// ZQ::Spot::NodeSrv NodeSrv(thpool);
	// ZQ::Spot::NodeSpot n(thpool, addrGroup, addrCtrlBind, nodeid, 
	// NodeSrv.getNodeMap());

	// NodeSrv.init(argc, argv, nodeid);
	// NodeSrv.start();
	
	/*
	if (::SetConsoleCtrlHandler((PHANDLER_ROUTINE)ConsoleHandler,TRUE)==FALSE)
	{
		printf("Unable to hook console handler!\n");
		return -1;
	}
	while (!bQuit)
	{
		printf("\"Ctrl-C\" at any time to exit the program....\r");
		::Sleep(1000);
	}
	*/
	// NodeSrv.stop();

	return 0;
}

BOOL WINAPI ConsoleHandler(DWORD CEvent)
{
    switch(CEvent)
    {
    case CTRL_C_EVENT:
    case CTRL_BREAK_EVENT:
    case CTRL_CLOSE_EVENT:
    case CTRL_LOGOFF_EVENT:
    case CTRL_SHUTDOWN_EVENT:
		bQuit = true;
        break;

    }
    return TRUE;
}
