// FtpCS.cpp : Defines the entry point for the console application.
//
#include "FileLog.h"
#include "contentImpl.h"
#include <stdio.h>
#include <direct.h>


BOOL WINAPI ConsoleHandler(DWORD event);
bool bQuit = false;

int main(int argc, char* argv[])
{
	char*	pLogPath=NULL;

	char path[MAX_PATH] = ".", *p=path;
	if (::GetModuleFileName(NULL, path, MAX_PATH-1)>0)
	{
		char* p = strrchr(path, FNSEPC);
		if (NULL !=p)
		{
			*p='\0';
			p = strrchr(path, FNSEPC);
			if (NULL !=p && (0==stricmp(FNSEPS "bin", p) || 0==stricmp(FNSEPS "exe", p)))
				*p='\0';
		}
	}

	strcat(path, FNSEPS);
	p = path+strlen(path);

	strcpy(p, "logs\\");
	mkdir(path);

	strcpy(p, "logs\\FTPCS.log");
	ZQ::common::FileLog FTPCSLogger(path, ZQ::common::Log::L_DEBUG, 1024*1024*20);

	strcpy(p, "logs\\FTPCS_events.log");
	ZQ::common::FileLog FTPCSEventLogger(path, ZQ::common::Log::L_INFO, 1024*1024*20);

	FTPCSLogger.setVerbosity(ZQ::common::Log::L_DEBUG);

	ZQ::common::setGlogger(&FTPCSLogger);

	int i =0;
	Ice::CommunicatorPtr ic = Ice::initialize(i, NULL);
	::ZQ::common::NativeThreadPool threadpool;
	ZQADAPTER_DECLTYPE adapter;

	try
	{
		//_adapter = _communicator->createObjectAdapterWithEndpoints(ADAPTER_NAME_Weiwoo, endpoint);		
		 adapter = ZQADAPTER_CREATE(ic, "FTPCS", DEFAULT_ENDPOINT_ContentStore, FTPCSLogger);
	}
	catch(Ice::Exception& ex)
	{
		FTPCSLogger(ZQ::common::Log::L_ERROR, CLOGFMT(FTPCS, "Create adapter failed with endpoint=%s and exception is %s"),
							DEFAULT_ENDPOINT_ContentStore, ex.ice_name().c_str());
		return -2;
	}

	{
		strcpy(p, "data\\FTPCS");
		::ZQTianShan::ContentStore::ContentStoreImpl::Ptr store = new ::ZQTianShan::ContentStore::ContentStoreImpl(FTPCSLogger,FTPCSEventLogger, threadpool, adapter, path);
		if (::SetConsoleCtrlHandler((PHANDLER_ROUTINE) ConsoleHandler, TRUE)==FALSE)
		{
			printf("Unable to install handler!                      \n");
			return -1;
		}

		printf("\"Ctrl-C\" at any time to exit the program.              \n");

		adapter->activate();

		while (!bQuit)
		{
			static const char* chs="-\\|/";
			static int chi=0;
			chi = ++chi %4;
			printf("\rFTPCS is now listening %c", chs[chi]);
			Sleep(200);
		}
	}

	ZQ::common::setGlogger(NULL);
	printf("\rFTPCS is quiting                   ");
	ic->destroy();
	Sleep(1000);

	printf("\rFTPCS stopped                    \n");
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
