// TestServer.cpp: implementation of the TestServer class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ChPub.h"
#include "TestServer.h"
#include "ChODFactory.h"
#include "ChODSvcEnv.h"
#include "ChannelPublisherImpl.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

// global console logger
ConsoleLog g_ConsoleLogger;

ConsoleLog::ConsoleLog()
{
	_verbosity = ZQ::common::Log::L_DEBUG;
	_hOutput = INVALID_HANDLE_VALUE;
}

ConsoleLog::~ConsoleLog()
{
	_hOutput = INVALID_HANDLE_VALUE;
}

int ConsoleLog::activate()
{
	// setup console
	BOOL succ;
	succ = AllocConsole();
	if(succ)
	{
		SetConsoleTitle("Channel Publisher Server");
		_hOutput = GetStdHandle(STD_OUTPUT_HANDLE);

		succ = SetStdHandle(STD_OUTPUT_HANDLE, _hOutput);
		if(!succ)
			return -1;
	}
	else
	{
		_hOutput = INVALID_HANDLE_VALUE;
	}

	return 0;
}

int ConsoleLog::deactivate()
{
	// close console
	if (_hOutput!=INVALID_HANDLE_VALUE)
	{
		CloseHandle(_hOutput);
		_hOutput = INVALID_HANDLE_VALUE;
		FreeConsole();
	}
	return 0;
}

void ConsoleLog::writeMessage(const char *msg, int level/* =-1 */)
{
	ZQ::common::MutexGuard	gd(_lock);

	DWORD lengthWritten = 0;
	DWORD lengthToWrite = strlen(msg);
	lengthToWrite = (lengthToWrite>2048)? 2048 : lengthToWrite;
	if (_hOutput!=INVALID_HANDLE_VALUE)
	{
		WriteConsole(_hOutput, msg, strlen(msg), &lengthWritten, NULL);
		WriteConsole(_hOutput, "\n", 1, &lengthWritten, NULL);
	}
	else
	{
		printf("%s\n", msg);
	}
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

TestServer::TestServer(::std::string endpoint)
	:_endpoint(endpoint)
{
	_bListening = false;
}

TestServer::~TestServer()
{
	signalStop();
	::Sleep(200);
}

int TestServer::run(void)
{
	g_ConsoleLogger.activate();

	// initialize communicator
	int i =0;
	::Ice::CommunicatorPtr ic = Ice::initialize(i, NULL);
	
	ZQChannelOnDemand::ChODSvcEnv::Configuration cfg;
	cfg._strChOdEndPoint = _endpoint;
	// create environment, this will create evictor, dictionary, factory...
	::ZQChannelOnDemand::ChODSvcEnv env(ic, cfg);
	env.init();

	 ::ZQChannelOnDemand::ChannelPublisherImpl* pTmp = new ::ZQChannelOnDemand::ChannelPublisherImpl(env);
//	 pTmp->setMonitorTraceFlag(true);
	::ChannelOnDemand::ChannelPublisherPtr pPublisher = pTmp;
	
	env._adapter->activate();
	_bListening = true;
	while(_bListening)
	{
		::Sleep(500);
	}

	ic->shutdown();
	ic->waitForShutdown();

	env.unInit();

	ic->destroy();

	g_ConsoleLogger.deactivate();
	return 0;
}

void TestServer::signalStop()
{
	_bListening = false;
}