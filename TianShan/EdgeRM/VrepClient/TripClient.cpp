// CRG_Client.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "ParseArgus.h"
#include "ClientSockets.h"
#include "FileLog.h"
#include <Windows.h>

Argus argus;

bool bQuit = false;
bool HandlerRoutine( DWORD dwCtrlType  )
{
	if(dwCtrlType == CTRL_C_EVENT)
	{
		bQuit = true;
		printf("In EchoServer.cpp HandlerRoutine() function£¬bQuit = %d\n",(int)bQuit);
		return TRUE;
	}
	else
	{
		return FALSE;
	}
};

int main(int argc, char* argv[])
{	
	/*
	if(!SetConsoleCtrlHandler((PHANDLER_ROUTINE)HandlerRoutine,TRUE))
		printf("error in set console ctrl handler\n");

	while(!bQuit)
	{
		ZQ::common::delay(500);
	}
	*/
	
	if(!parseArgus(argc,argv,argus))
	{
		printf("parseArgus error\n");
		return 0;
	}

	ZQ::common::NativeThreadPool procThPool(argus.procThPoolSize);
	ZQ::common::FileLog  gLog(argus.logPath,7);
	ZQ::common::setGlogger(&gLog);
	
	ClientSockets	mainSocket(gLog,procThPool);
	
	if(!mainSocket.setSocketPara(argus.bindIP,argus.bindPort))
	{
		printf("setSocketPara error\n");
		return 0;
	}

	if(!mainSocket.createMsg(argus.holdTime))
	{
		printf("create message error\n");
		return 0;
	}

	printf("start connect %s:%d with %d connections...\n",argus.bindIP,argus.bindPort,argus.maxConnection);

	mainSocket.start(argus.recvThPoolSize,argus.maxConnection,argus.interval);

	Sleep((argus.holdTime+5)*1000);
	mainSocket.stop();
	printf("service stopped\n");
	ZQ::common::setGlogger(NULL);
	Sleep(3000);

	return 0;
}