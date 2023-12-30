#include "RpcGeekServer.h"
#include "Guid.h"
#include "Locks.h"

#define test2 main

class Sample : public ZQ::RpcGeek::SkelHelper
{
public:
	Sample(ZQ::RpcGeek::Server& skel) : SkelHelper(skel) {}
	virtual ~Sample() {}
	
	BEGIN_SKELHELPER_METHODS()
		SKELHELPER_METHOD(Sample, add)
		SKELHELPER_METHOD(Sample, sub)
		SKELHELPER_METHOD(Sample, mul)
		//	REG_METHOD(Callee, div)
	END_SKELHELPER_METHODS()
		
	virtual void add(ZQ::common::Variant& params, ZQ::common::Variant& result, const char* host)
	{
		result = ZQ::common::Variant((int)params[0] + (int)params[1]);
	}

	virtual void sub(ZQ::common::Variant& params, ZQ::common::Variant& result, const char* host)
	{
		result = ZQ::common::Variant((int)params[0] - (int)params[1]);
	}

	virtual void mul(ZQ::common::Variant& params, ZQ::common::Variant& result, const char* host)
	{
		result = ZQ::common::Variant((int)params[0] * (int)params[1]);
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
	ZQ::RpcGeek::Server skel("http://localhost:2999/RPC2");
	Sample sample(skel);
	
	skel.serv();
	
	if (::SetConsoleCtrlHandler((PHANDLER_ROUTINE)ConsoleHandler,TRUE)==FALSE)
	{
		printf("Unable to install handler!\n");
		return -1;
	}
	printf("Processing requests..., \"Ctrl-C\" at any time to exit the program.\n");
	
	while (!bQuit)
	{
		Sleep(100);
	}
	return 0;
}

int test2(int argc, char* argv[])
{
	ZQ::RpcGeek::Server skel("http://localhost:2999/RPC2");
	ZQ::RpcGeek::ServerObjectHelper Helper(skel);

	skel.serv();
	
	if (::SetConsoleCtrlHandler((PHANDLER_ROUTINE)ConsoleHandler,TRUE)==FALSE)
	{
		printf("Unable to install handler!\n");
		return -1;
	}
	
	while (!bQuit)
	{
		printf("\"Ctrl-C\" at any time to exit the program. object count:%d\r", Helper.size());
		Sleep(100);
	}
	return 0;
}

