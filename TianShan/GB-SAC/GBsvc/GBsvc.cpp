#include "GBsvcConfig.h"
#include "GBsvc.h"
#include "confighelper.h"
#include "MiniDump.h"

#include "SocketClientContext.h"

#ifdef ZQ_OS_MSWIN
DWORD gdwServiceType = 1 ;
DWORD gdwServiceInstance = 1;
#endif// ZQ_OS_MSWIN


ZQTianShan::GBServerNS::GBService   g_GBService;
ZQ::common::BaseZQServiceApplication  *Application = &g_GBService;

ZQ::common::Config::Loader<ZQTianShan::GBServerNS::GBServerConfig > gConfig("GBsvc.xml");
ZQ::common::Config::ILoader *configLoader = &gConfig;

static ZQ::common::MiniDump g_crashDump;


namespace ZQTianShan {

namespace GBServerNS {

GBService::GBService()
:_pGBSvcEnv(NULL)
{				
}

GBService::~GBService ()
{
	if (NULL != _pGBSvcEnv)
	{
		delete _pGBSvcEnv;
		_pGBSvcEnv = NULL;
	}
}

HRESULT GBService::OnStart(void)
{
	if (NULL != _pGBSvcEnv)
		_pGBSvcEnv->start();

	return BaseZQServiceApplication::OnStart();
};

HRESULT GBService::OnStop(void)
{
	if (NULL != _pGBSvcEnv)
		_pGBSvcEnv->resume();

	return BaseZQServiceApplication::OnStop();
};

HRESULT GBService::OnInit(void)
{
	if(NULL == _pGBSvcEnv)
		_pGBSvcEnv = new ServiceEnv(gConfig._gbSvcBase._localIp, gConfig._gbSvcBase._localPort, *m_pReporter);

	if (!initializeCrashDumpLocation())
	{
#ifdef ZQ_OS_MSWIN
		logEvent(ZQ::common::Log::L_ERROR,_T("failed to initialize crash dump location"));
#endif
		return S_FALSE;
	}

	return BaseZQServiceApplication::OnInit();
};

HRESULT GBService::OnUnInit(void)
{
	if (NULL != _pGBSvcEnv)
	{
		delete _pGBSvcEnv;
		_pGBSvcEnv = NULL;
	}

	return BaseZQServiceApplication::OnUnInit();
};


bool GBService::initializeCrashDumpLocation(void)
{
	try
	{
		if(!GBsvcUtil::fsCreatePath(gConfig._crashDump._path))
		{
			(*m_pReporter)(ZQ::common::Log::L_ERROR,CLOGFMT(GBService,"can't create path [%s] for mini dump "),
				gConfig._crashDump._path);
			return false;
		}

		g_crashDump.setDumpPath((char *)gConfig._crashDump._path.c_str());
		g_crashDump.enableFullMemoryDump(gConfig._crashDump._enabled);
		g_crashDump.setExceptionCB(GBsvcUtil::CrashExceptionCallBack);
	}
	catch (...)
	{
		(*m_pReporter)(ZQ::common::Log::L_ERROR, CLOGFMT(GBService, "unexpected exception caught when initializeCrashDumpLocation"));
		return false;
	}

	return true;
}

ServiceEnv::ServiceEnv(std::string localIp, std::string localPort, ZQ::common::Log& svcLog)
:_localIp(localIp), _localPort(localPort), _svcLog(svcLog)
{	
}

int ServiceEnv::run()
{
	WSADATA wsaData;
	if (NO_ERROR != WSAStartup(MAKEWORD(2, 2), &wsaData))
	{
		_svcLog(ZQ::common::Log::L_ERROR, CLOGFMT(ServiceEnv,"ServiceAccept run WSAStartup failed, errorCode[%u]"), WSAGetLastError());	
		return 1;
	}

	CreateSvcSocket creatHandle(_svcLog);
	if(!creatHandle.createSocket(AF_INET, SOCK_STREAM, IPPROTO_TCP) ||
		!creatHandle.bind(_localIp, _localPort)	|| 
		!creatHandle.listen() )
	{
		_svcLog(ZQ::common::Log::L_ERROR, CLOGFMT(ServiceEnv,"ServiceAccept run quit, errorCode[%u]"), WSAGetLastError());	
		return false;
	}

	unsigned long acceptCount = 0;
	unsigned long acceptFailed = 0;
	while (_running)
	{
		ClientSocketContext *pClientContext = creatHandle.accept();
		if (NULL == pClientContext)
		{
			++acceptFailed;
			_svcLog(ZQ::common::Log::L_ERROR, CLOGFMT(ServiceEnv,"ServiceAccept run accept acceptFailed[%d]"), acceptFailed);
			continue;
		}

		++acceptCount;
		_svcLog(ZQ::common::Log::L_DEBUG, CLOGFMT(ServiceEnv,"ServiceAccept run accept acceptCount[%d]"), acceptCount);
	}

	_svcLog(ZQ::common::Log::L_ERROR, CLOGFMT(ServiceEnv,"ServiceAccept run quit normally"));
}

ServiceEnv::~ServiceEnv()
{
	_running = false;
}

void WINAPI GBsvcUtil::CrashExceptionCallBack(DWORD ExceptionCode, PVOID ExceptionAddress)
{
	DWORD dwThreadID = GetCurrentThreadId();
	ZQ::common::FileLog& svcLog = *(g_GBService.m_pReporter);

	svcLog( ZQ::common::Log::L_CRIT, 
		_T("Crash exception callback called,ExceptionCode 0x%08x, ExceptionAddress 0x%08x, Current Thread ID: 0x%04x"),
		ExceptionCode, ExceptionAddress, dwThreadID);

	svcLog.flush();	
}

void GBsvcUtil::replaceCharacter( int iLen , char* p , const char* pSrc , int& iPos, bool& bLastSlash )
{
	for ( int i = 0 ;i < iLen ; i ++ )
	{
		if( pSrc[i] == '/' )
		{
			if( bLastSlash	)
			{
				continue;			
			}
			else
			{
				p[iPos++] = '\\';
			}
			bLastSlash = true;
		}
		else if ( bLastSlash && pSrc[i] == '\\'  )
		{
			continue;
		}
		else
		{
			bLastSlash = pSrc[i] == '\\';
			p[iPos++]=pSrc[i];
		}
	}
}

std::string GBsvcUtil::fsFixupPath( const std::string& strPath)
{
	int iLen = static_cast<int>(strPath.length());
	const char* pSrc = strPath.c_str();
	char* p = new char[iLen + 1 ];
	p[iLen] = 0;
	assert( p != NULL );
	bool bLastSlash = false;
	int iPos = 0;
	replaceCharacter(iLen,p,pSrc,iPos, bLastSlash);
	p[iPos] = 0;
	std::string result(p);
	delete[] p;
	return result;
}

bool GBsvcUtil::fsCreatePath( const std::string& strPath)
{
#ifdef ZQ_OS_MSWIN
	if( !CreateDirectoryA(fsFixupPath(strPath).c_str(),NULL) )
	{
		if ( ERROR_ALREADY_EXISTS == GetLastError() )
		{
			return true;
		}
		else
		{
			return false;
		}
	}
#endif

	return true;
}

}//GBServerNS
}//	ZQTianShan	
