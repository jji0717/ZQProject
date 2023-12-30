#include "stdafx.h"
#include "JMSdispatchdll.h"
#include "DisPatchMain.h"
typedef IceUtil::Handle<DODAppThread> DODAppMainThreadPtr;
DODAppMainThreadPtr DodMainThread = NULL;
ZQ::common::Log* g_logger;
Ice::CommunicatorPtr g_ic = NULL;
void writeLog(ZQ::common::Log::loglevel_t level, char* fmt, ...)
{
	if(g_logger == NULL)
	  return;

	char logMsg[1024];
	memset(logMsg, 0x00, 1024*sizeof(char));

	va_list args;
	va_start(args, fmt);
	_vsnprintf(logMsg, 1024,fmt, args);
	va_end(args);
	
	char logMsgWID[1024];
//	_snprintf(logMsgWID, 510, "0x%08X, %s", GetCurrentThreadId(), logMsg);
	_snprintf(logMsgWID, 1024, "%s", logMsg);
	(*g_logger)(level, logMsgWID);
}

void  GetErrorDescription(int nErrorcode, char *StrError)
{
	 LPVOID lpMsgBuf; //Windows will allocate 
	 ::FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | 
		 FORMAT_MESSAGE_FROM_SYSTEM,0, nErrorcode, 
	 MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),(LPTSTR)&lpMsgBuf, 0, NULL );
	 
	 strcpy(StrError,(LPTSTR)lpMsgBuf);
	::LocalFree( lpMsgBuf );
}
extern "C" MYJMSDISPATCH_API
BOOL JmsPortInitialize(Ice::CommunicatorPtr ic,ZQ::common::Log* pLog)
{
	g_logger = pLog;
	g_ic = ic;
	DodMainThread = new DODAppThread();
    DodMainThread->start();
    return TRUE;
}

extern  "C" MYJMSDISPATCH_API
BOOL JmsPortUnInitialize()
{
	if(DodMainThread)
	{
		DodMainThread->stop();
		
        DodMainThread  =NULL;		
	}

   return TRUE;
}