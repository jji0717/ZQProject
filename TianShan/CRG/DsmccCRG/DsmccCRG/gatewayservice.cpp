#include <ZQ_common_conf.h>
#include <minidump.h>
#include <TianShanIceHelper.h>
#include <IceLog.h>
#include "environment.h"
#include "gatewayservice.h"
#include "gatewayconfig.h"


ZQ::CLIENTREQUEST::GatewayService g_GatewayService;
ZQ::common::BaseZQServiceApplication  *Application = &g_GatewayService;

namespace ZQ { namespace CLIENTREQUEST {
ZQ::common::Config::Loader<ZQ::CLIENTREQUEST::Config::Gateway> gwConfig("dsmccCrg.xml");
}}

ZQ::common::Config::ILoader *configLoader = &ZQ::CLIENTREQUEST::gwConfig;

extern const char* DUMP_PATH;

#ifdef ZQ_OS_MSWIN
DWORD gdwServiceType = 1 ;
DWORD gdwServiceInstance = 1;

// crash dump
ZQ::common::MiniDump _crashDump;
static void WINAPI CrashExceptionCallBack(DWORD ExceptionCode, PVOID ExceptionAddress)
{
	DWORD dwThreadID = GetCurrentThreadId();	
	glog( ZQ::common::Log::L_ERROR,  "Crash exception callback called,ExceptonCode 0x%08x, ExceptionAddress 0x%08x, Current Thread ID: 0x%04x",ExceptionCode, ExceptionAddress, dwThreadID);	
	glog.flush();
}

static bool validatePath( const char *     szPath )
{
	if (-1 != ::GetFileAttributesA(szPath))
		return true;

	DWORD dwErr = ::GetLastError();
	if ( dwErr == ERROR_PATH_NOT_FOUND || dwErr == ERROR_FILE_NOT_FOUND )
	{
		if (!::CreateDirectoryA(szPath, NULL))
		{
			dwErr = ::GetLastError();
			if ( dwErr != ERROR_ALREADY_EXISTS)
			{
				return false;
			}
		}
	}
	else
	{
		return false;
	}

	return true;
}
#endif

namespace ZQ{ namespace CLIENTREQUEST {

GatewayService::GatewayService()
:mGatewayEnv(0),
mGwCenter(0)
{
}

GatewayService::~GatewayService()
{
}

bool GatewayService::initIceRuntime()
{
	Ice::InitializationData		iceInitData;
	int i = 0;
	iceInitData.properties =Ice::createProperties( i , NULL );

	//set ice properties
	
	std :: map<std::string, std::string>::const_iterator it = gwConfig.iceProps.begin();
	for( ; it != gwConfig.iceProps.end() ; it ++ )
	{		
		iceInitData.properties->setProperty( it->first , it->second );
		glog(ZQ::common::Log::L_INFO,CLOGFMT(GatewayService,"Set ice property [%60s] = [%s]"),
			it->first.c_str() , it->second.c_str() );
	}


	if( gwConfig.icetraceenabled >= 1 )
	{
		std::string path = ZQTianShan::Util::fsConcatPath(m_wsLogFolder,"DsmccCRG.IceTrace.log");
		try
		{			
			mIceTraceLogger.open( path.c_str() ,gwConfig.icetracelevel,ZQLOG_DEFAULT_FILENUM,gwConfig.icetracelogsize ,	10240,	2);
		}
		catch( const ZQ::common::FileLogException& ex)
		{
			glog(ZQ::common::Log::L_ERROR,CLOGFMT(GatewayService,"failed to open ice trace log file[%s] because [%s]"),
				path.c_str(),ex.what() );
			return false;
		}
		iceInitData.logger = new TianShanIce::common::IceLogI( &mIceTraceLogger );
		assert( iceInitData.logger );

	}

	mIc	=	Ice::initialize( i , NULL , iceInitData );

	try
	{
		mAdapter = ZQADAPTER_CREATE(mIc,"Gateway",gwConfig.binding.c_str(),glog);
	}
	catch( const Ice::Exception &ex)
	{
		glog(ZQ::common::Log::L_ERROR,CLOGFMT(GatewayService,"initIceRuntime() caught [%s] while setup adapter with binding[%s]"),
			ex.ice_name().c_str(), gwConfig.binding.c_str() );
		return false;
	}

	return ( mAdapter != 0 );
}

HRESULT GatewayService::OnInit(void)
{
#ifdef ZQ_OS_MSWIN
	// step 2: crash dump
	if(gwConfig.crashdumpenabled)
	{
		if(!validatePath(gwConfig.crashdumppath.c_str()))
		{
			glog(::ZQ::common::Log::L_ERROR, CLOGFMT(GatewayService, "OnInit() bad dump path [%s]"), gwConfig.crashdumppath.c_str());
			return S_FALSE;
		}
		// enable crash dump

		_crashDump.setDumpPath((char*)gwConfig.crashdumppath.c_str());
		_crashDump.enableFullMemoryDump(true);
		_crashDump.setExceptionCB(CrashExceptionCallBack);
	}
#else
	DUMP_PATH = gwConfig.crashdumppath.c_str();
#endif

	if( !initIceRuntime() )
	{
		glog(ZQ::common::Log::L_ERROR,CLOGFMT(GatewayService,"OnInit() failed to initialize ice runtime environment"));
		return S_FALSE;
	}

	mGatewayEnv = new Environment(*m_pReporter,mAdapter);
	mGwCenter	= new GatewayCenter(*mGatewayEnv);
	if (NULL != mGatewayEnv)
	    mGatewayEnv->registerSnmp(mGwCenter);

	glog(ZQ::common::Log::L_INFO,CLOGFMT(GatewayService,"OnInit() environment[%s], center[%s]"), (NULL != mGatewayEnv ? "Succeed" : "NULL"), (NULL != mGwCenter ? "Succeed" : "NULL"));
	return BaseZQServiceApplication::OnInit();
}

HRESULT GatewayService::OnStart(void)
{
	if(!mGwCenter)
	{
		glog(ZQ::common::Log::L_ERROR,CLOGFMT(GatewayService,"OnStart() failed to initialize service, refuse to start"));
		return S_FALSE;
	}

	if(!mGwCenter->start(m_wsLogFolder,gwConfig.dbruntimepath.c_str() ))
	{
		glog(ZQ::common::Log::L_ERROR,CLOGFMT(GatewayService,"OnStart() failed to start gateway service"));
		return S_FALSE;
	}
	mAdapter->activate();
	return BaseZQServiceApplication::OnStart();
}

HRESULT GatewayService::OnStop(void)
{
	glog(ZQ::common::Log::L_INFO,CLOGFMT(GatewayService,"OnStop() gateway is stopping"));
	if( mAdapter)
	{
		mAdapter->deactivate();
	}
	if( mAdapter)
	{
		mAdapter = 0;
	}
	if( mIc )
	{
		try
		{
			mIc->destroy();
		}
		catch(...)
		{
		}
		mIc = NULL;		
	}
	if(mGwCenter)	
		mGwCenter->stop();
	return BaseZQServiceApplication::OnStop();
}

HRESULT GatewayService::OnUnInit(void)
{	
	if( mGwCenter )
		delete mGwCenter;
	if( mGatewayEnv)
		delete mGatewayEnv;
	glog(ZQ::common::Log::L_INFO,CLOGFMT(GatewayService,"OnUnInit() gateway stopped"));
	return BaseZQServiceApplication::OnUnInit();
}


}}
