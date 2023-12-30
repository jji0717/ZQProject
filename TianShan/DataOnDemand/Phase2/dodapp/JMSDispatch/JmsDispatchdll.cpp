#include "stdafx.h"
#include "JMSdispatchdll.h"
#include "DisPatchMain.h"
DODAppThread* DodMainThread = NULL;
ZQ::common::Log* g_logger;
Ice::CommunicatorPtr g_ic = NULL;
void writeLog(ZQ::common::Log::loglevel_t level, char* fmt, ...)
{
	if(g_logger == NULL)
	  return;

	char logMsg[2048];
	memset(logMsg, 0x00, 2048*sizeof(char));

	va_list args;
	va_start(args, fmt);
	_vsnprintf(logMsg, 2048,fmt, args);
	va_end(args);
	
	char logMsgWID[2048];
//	_snprintf(logMsgWID, 510, "0x%08X, %s", GetCurrentThreadId(), logMsg);
	_snprintf(logMsgWID, 2048, "%s", logMsg);
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
BOOL JmsPortInitialize(std::string JbossIpPort,
					   std::string ConfigQueueName,
					   std::string CachePath,
					   long  ConfigMsgTimeOut,
					   long  UsingJboss,
					   std::string DODEndPoint,
					   Ice::CommunicatorPtr ic,
					   ZQ::common::Log* pLog)
{
	g_logger = pLog;
	g_ic = ic;
	DodMainThread = new DODAppThread();
	DodMainThread->m_jmspar.CacheFolder = CachePath;
    DodMainThread->m_jmspar.JbossIpport = JbossIpPort;
	DodMainThread->m_jmspar.comfigTimeOut = ConfigMsgTimeOut;
	DodMainThread->m_jmspar.UsingJboss = UsingJboss;
	DodMainThread->m_jmspar.ConfigQueueName = ConfigQueueName;
	DodMainThread->m_jmspar.DODEndPoint = DODEndPoint;
    DodMainThread->start();
    return TRUE;
}

extern  "C" MYJMSDISPATCH_API
BOOL JmsPortUnInitialize()
{
	try
	{
		if(DodMainThread)
		{
			DodMainThread->stop();
			delete DodMainThread;
			DodMainThread  =NULL;		
		}
	}
	catch(...)
	{
	}
   return TRUE;
}