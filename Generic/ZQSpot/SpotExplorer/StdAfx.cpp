// stdafx.cpp : source file that includes just the standard includes
//  stdafx.pch will be the pre-compiled header
//  stdafx.obj will contain the pre-compiled type information

#include "stdafx.h"

#ifdef _ATL_STATIC_REGISTRY
#include <statreg.h>
#include <statreg.cpp>
#endif
#include <atlimpl.cpp>
// #include "../MPFNode.h"
// #include "../ZQSpotAgent.h"
// #include "../NodeMgr.h"
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
ZQ::Spot::SpotEnv *g_pspotEnv;
Ice::CommunicatorPtr g_ic;
char* g_Ip = NULL;
char* g_Port = NULL;

#include "variant.h"

using namespace ZQ::common;

int Init(int argc, char* argv[])
{
	if (argc != 3) {
		printf("invalid command line.\n");
		return -1;
	}
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
	g_pspotEnv = new SpotEnv(init);
	if(g_pspotEnv)
	{
		if (!g_pspotEnv->initialize(argc, argv))
			return 0;
		g_ic = g_pspotEnv->getCommunicator();
		TestIfPtr test1 = new TestIfI;
		g_pspotEnv->exportInterface(test1, "TestIf");   
	
		Ice::Application::shutdownOnInterrupt();
		
		TestIfPrx test = TestIfPrx::checkedCast(
	     g_pspotEnv->openInterface("TestIf"));	
		return 0;
	}
		return -1;
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

DWORD WINAPI DataChange(LPVOID pParam)
{
	while (true) {
		try {
			Test::TestIfPrx test = Test::TestIfPrx::checkedCast(
					g_pspotEnv->openInterface("TestIf"));
			test->TestFn("1111");
		}
		catch (const Ice::Exception & e) {

			std::cerr << e << std::endl;
			assert(FALSE);
			AtlTrace("123456+");
		}
		
		Sleep(3000);
	}
	return 0;
}