// ERMIServer.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "ERMIMsgHanler.h"
#include "TianShanDefines.h"
#include "RtspDak.h"
#include "RtspEngine.h"

using namespace ZQTianShan::EdgeRM;
using namespace ZQ::common;

bool _bQuit = false;

bool WINAPI ConsoleHandler(DWORD CEvent)
{
	switch(CEvent)
	{
	case CTRL_C_EVENT:
	case CTRL_BREAK_EVENT:
	case CTRL_CLOSE_EVENT:	
		printf("exit program\n");
		_bQuit = true;
		break;
	case CTRL_LOGOFF_EVENT:
		break;
	case CTRL_SHUTDOWN_EVENT:
		break;
	}
	return true;
}

int main(int argc, char* argv[])
{
	if(argc < 3)
	{
		printf("command Line parameter: <IP> <Port>\n");
		return 0;
	}
	char* ipV4 = argv[1];
	char* ipV4Port = argv[2];

	std::string logfilepath = ZQTianShan::getProgramRoot();
	logfilepath += "/ERMIServer.log";

	ZQ::common::FileLog ermiServerLog(logfilepath.c_str(), ZQ::common::Log::L_DEBUG);

	ZQRtspCommon::IRtspDak*       _rtspDak;
	ZQRtspEngine::RtspEngine*     _rtspEngine;	
	
	ERMIMsgHanler*  ermiMsgHandler = new (std::nothrow) ::ZQTianShan::EdgeRM::ERMIMsgHanler(ermiServerLog);

	_rtspDak = new (std::nothrow) ZQRtspCommon::RtspDak(ermiServerLog, 50, 50);
	_rtspDak->registerHandler(ermiMsgHandler);

	if (!_rtspDak->start())
	{
		printf("failed to start rtspDak \n");
		return 0;
	}

	_rtspEngine = new ZQRtspEngine::RtspEngine(ermiServerLog, _rtspDak);

	if (!_rtspEngine->startTCPRtsp(ipV4, "", ipV4Port))
	{
		printf("failed to start TCP Rtsp Engine \n");
		_rtspDak->stop();
		return 0;
	}

	if(SetConsoleCtrlHandler((PHANDLER_ROUTINE)ConsoleHandler,true) == false)  
		printf("unable to register console handler\n");

	while(!_bQuit)
	{
		Sleep(500);
	}

	try
	{
		if(_rtspDak)
		{
			_rtspDak->stop();
			_rtspDak->release();
		}
	}
	catch (...){
	}
	return 1;
}


