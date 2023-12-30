#include "RpcGeekClient.h"
#include "Guid.h"
#include <iostream>    // for cout, endl
#include <strstream>   // for sstream

#define test3 main
#define TEST_SERVER_ENDPOINT "http://localhost:2999/RPC2"

class myClient : public ZQ::RpcGeek::Client
{
public:
	myClient(const char* serverUrl, const char* clientName =CLIENT_NAME, const char* protocolVer =PROTOCOL_VERSION)
		:Client(serverUrl){}
	~myClient(){}

	virtual void OnDefaultAsynResponse(const char* methodName, ZQ::common::Variant& params, ZQ::common::Variant& result, const int faultcode)
	{
		int v = result;
		printf ("\n%s(%d, %d) = %3d", methodName, (int)params[0], (int)params[1], v);
	}

};


bool bQuit =false;
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

int test1(int argc, char* argv[])
{
	myClient cs("http://localhost:2999/RPC2");
	
	if (::SetConsoleCtrlHandler((PHANDLER_ROUTINE)ConsoleHandler,TRUE)==FALSE)
	{
		printf("Unable to install handler!\n");
		return -1;
	}
	printf("Sending requests..., \"Ctrl-C\" at any time to exit the program.\n");

	int i =0;
	while (!bQuit)
	{
		i++;
		i &=0xffffff;
		ZQ::common::Variant param, result;
		int a= ((int)(94.23556 *i)) %20;
		int b= ((int)(74.6433 *i)) %30;
		
		param.set(0, ZQ::common::Variant(a));
		param.set(1, ZQ::common::Variant(b));
		
		cs.call_async("Sample.add", param);
		cs.call_async("Sample.sub", param);
		cs.call_async("Sample.mul", param);
		Sleep(50);
	}

	return 0;
}

int test2(int argc, char* argv[])
{
	myClient cs("http://localhost:2999/RPC2");
	
	if (::SetConsoleCtrlHandler((PHANDLER_ROUTINE)ConsoleHandler,TRUE)==FALSE)
	{
		printf("Unable to install handler!\n");
		return -1;
	}
	printf("Sending requests..., \"Ctrl-C\" at any time to exit the program.\n");

	int i =0;
	while (!bQuit)
	{
		i++;
		i &=0xffffff;
		ZQ::common::Variant params, result;
		int a= ((int)(94.23556 *i)) %20;
		int b= ((int)(74.6433 *i)) %30;
		
		params.set(0, ZQ::common::Variant(a));
		params.set(1, ZQ::common::Variant(b));
		
		cs.call_sync("Sample.add", params, result);
		int v = result;
		printf ("\n%s(%d, %d) = %3d", "Sample.add", (int)params[0], (int)params[1], v);
		cs.call_sync("Sample.sub", params, result);
		v = (int)result;
		printf ("\n%s(%d, %d) = %3d", "Sample.sub", (int)params[0], (int)params[1], v);
		cs.call_sync("Sample.mul", params, result);
		v = (int)result;
		printf ("\n%s(%d, %d) = %3d", "Sample.mul", (int)params[0], (int)params[1], v);
		Sleep(50);
	}

	return 0;
}

int test3(int argc, char* argv[])
{
	ZQ::RpcGeek::ProxyObject pl1(TEST_SERVER_ENDPOINT, NULL, "55647F54-4179-4B08-981B-02C908555D4A");
	for (int i=0; i< 100; i++)
	{
		ZQ::RpcGeek::ProxyObject pl(TEST_SERVER_ENDPOINT, NULL); //, pl3(cs, NULL,"55647F54-4179-4B08-981B-02C908555D4A");
		
		::Sleep(50);
	}
	
	return 0;
}

