// EchoServer.cpp : Defines the entry point for the console application.
//

#include  "stdafx.h"
#include  "TripServerService.h"
#include  "FileLog.h"
#include  "NativeThreadPool.h"

bool bQuit = false;
BOOL WINAPI  HandlerRoutine( DWORD dwCtrlType  )
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

// *********************************************
    //ZQ::common::NativeThreadPool  _gPool(20);
// *********************************************

int main(int argc, char* argv[])
{
	if( argc < 3 )
	{
		printf("usage: %s localIp localPort\n");
		return 0;
	}
	SetConsoleCtrlHandler(HandlerRoutine,TRUE);
	std::string bindIp = argv[1];
	std::string bindPort = argv[2];

    ZQ::common::FileLog echoLog("c:\\TripSocketServer.log", 7); 
    ZQ::common::setGlogger(&echoLog);

	TripSocketServer server(echoLog,20,20);

	if(!server.start())
	{
		printf("failed to start Service\n");
		return -1;
	}
	printf("service starting...\n");
	if(!server.addListener(bindIp,bindPort))
	{
		printf("failed to add listener at %s:%s\n",bindIp.c_str(), bindPort.c_str());
		return -1;
	}
    
	printf("service is working, listening at %s:%s...\n",bindIp.c_str(), bindPort.c_str());
	
	while(!bQuit)
	{
		ZQ::common::delay(500);
	}

	server.stop();

	ZQ::common::setGlogger(NULL);
	printf("service stopped\n");
	
	return 0;
}

