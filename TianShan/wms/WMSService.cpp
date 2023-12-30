// WMSService.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <NativeThread.h>
#include "TsStreamerImpl.h"
#include "HttpService.h"

using namespace TianShanIce;
using namespace ZQ;

class IceMainThread: public common::NativeThread {
public:
	virtual bool init()
	{
		CoInitializeEx(NULL, COINIT_MULTITHREADED);
		return true;
	}

	virtual int run()
	{
		int argc = 0;
		Ice::CommunicatorPtr ic = Ice::initialize(argc, NULL);
		Streamer::IceExporter exporter(ic, ICE_PORT);
		if (!exporter.init()) {
			printf("exporter initilization failed.\n");
			return -1;
		}

		Streamer::WMSStreamerServiceImpl* wmsService = 
			new Streamer::WMSStreamerServiceImpl(exporter, "WMSTest");
		if (!wmsService->init()) {
			printf("wmsService initilization failed.\n");
			return -1;
		}
		exporter.export(wmsService, "StreamerService");
		Ice::ObjectPrx prx = exporter.export(wmsService, 
			"WMSStreamerService");
		Ice::Application::shutdownOnInterrupt();
		ic->waitForShutdown();
		return 0;
	}

	virtual void final()
	{
		CoUninitialize();
	}
};

void WaitForStop()
{
	Sleep(INFINITE);
}


common::NativeThreadPool wmsServiceThreadPool(20);
int main(int argc, char* argv[])
{
	IceMainThread	iceMainThread;
	sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = htons(8080);
	HttpService	httpService(wmsServiceThreadPool, (sockaddr& )addr);
	iceMainThread.start();
	httpService.start();
	iceMainThread.waitHandle(INFINITE);
	httpService.waitHandle(INFINITE);
	// WaitForStop();
	return 0;
}
