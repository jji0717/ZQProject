#include "C2SSService.h"
#include "IceLog.h"

#include <string>
#include <SystemInfo.h>

#ifdef ZQ_OS_MSWIN
#include <MiniDump.h>
#endif

namespace ZQ{
	namespace StreamService{
		ZQ::StreamService::SsServiceImpl* pServiceInstance = NULL;
		SsEnvironment*	gSsEnvironment = NULL;
	}}

ZQTianShan::C2SS::C2SSService					gC2SSServiceInstance;
ZQ::common::BaseZQServiceApplication		*Application=&gC2SSServiceInstance;

ZQ::common::Config::Loader<ZQTianShan::C2SS::C2SSCfg> gC2SSConfig("C2SS.xml");
ZQ::common::Config::ILoader					*configLoader = &gC2SSConfig;

ZQTianShan::C2SS::ST_C2SS::C2SSHolder	*pC2SSBaseConfig = NULL;

#ifdef ZQ_OS_MSWIN
DWORD gdwServiceType = 1;
DWORD gdwServiceInstance = 0;

static ZQ::common::MiniDump			_crashDump;
//void WINAPI CrashExceptionCallBack(DWORD ExceptionCode, PVOID ExceptionAddress);
void WINAPI CrashExceptionCallBack(DWORD ExceptionCode, PVOID ExceptionAddress)
{
	DWORD dwThreadID = GetCurrentThreadId();

	glog( ZQ::common::Log::L_CRIT, 
		_T("Crash exception callback called,ExceptionCode 0x%08x, ExceptionAddress 0x%08x, Current Thread ID: 0x%04x"),
		ExceptionCode, ExceptionAddress, dwThreadID);
	glog.flush();	
}
#else
extern const char* DUMP_PATH;
#endif


namespace ZQTianShan{
	namespace C2SS{
C2SSService::C2SSService(void)
{

#ifdef ZQ_OS_MSWIN
	strcpy(servname, "C2SS");
	strcpy(prodname, "TianShan");
#endif
	mIc				=	NULL;
	mpServiceImpl	=	NULL;
	// nssSnmpAgent    =   NULL;
}

C2SSService::~C2SSService(void)
{
}

bool C2SSService::initializeCrashDumpLocation( )
{
#ifdef ZQ_OS_MSWIN
	try
	{
		if(! ZQTianShan::Util::fsCreatePath( gC2SSConfig._szCrashDumpPath) )
		{
			glog(ZQ::common::Log::L_ERROR,CLOGFMT(C2SSService,"can't create path [%s] for mini dump "),
				gC2SSConfig._szCrashDumpPath);
			return false;
		}	
		_crashDump.setDumpPath((char *)gC2SSConfig._szCrashDumpPath);
		_crashDump.enableFullMemoryDump(gC2SSConfig._crashDumpEnabled);
		_crashDump.setExceptionCB(CrashExceptionCallBack);
	}
	catch (...)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(C2SSService, "unexpected exception caught when initializeCrashDumpLocation"));
		return false;
	}
#else
	DUMP_PATH = gC2SSConfig._szCrashDumpPath;
#endif
	return true;
}

std::string	C2SSService::getLoggerFolder( ) const
{
#ifdef ZQ_OS_MSWIN
	std::string folderPath = m_wsLogFolder;
#else
	std::string folderPath = _logDir;
#endif
	if ( folderPath.at(folderPath.length()-1) != FNSEPC)
	{
		folderPath += FNSEPS;
	}
	return folderPath;
}

bool C2SSService::initializeLogger( )
{
#ifdef ZQ_OS_MSWIN
	::std::string strLogBaseFolder = m_wsLogFolder;
	std::string svc_name = servname;
#else
	::std::string strLogBaseFolder = _logDir;
	std::string svc_name = getServiceName();
#endif
	::std::string strNSSLog	= strLogBaseFolder + FNSEPS + svc_name + ".sess.log";

	try
	{
#ifdef ZQ_OS_MSWIN
		int32 iLogLevel			= m_dwLogLevel;
		int32 iLogMaxCount		= m_dwLogFileCount;
		int32 iLogSize			= m_dwLogFileSize;
		int32 iLogBufferSize	= m_dwLogBufferSize;
		int32 iLogFlushTimeout	= m_dwLogWriteTimeOut;
#else
		int32 iLogLevel			= _logLevel;
		int32 iLogMaxCount		= ZQLOG_DEFAULT_FILENUM;
		int32 iLogSize			= _logSize;
		int32 iLogBufferSize	= _logBufferSize;
		int32 iLogFlushTimeout	= _logTimeout;
#endif

		mSessionLogger.open(strNSSLog.c_str(),
			iLogLevel,
			iLogMaxCount,
			iLogSize,
			iLogBufferSize,
			iLogFlushTimeout);

		if( gC2SSConfig._iceTraceLogEnabled >= 1 )
		{
			strNSSLog = strLogBaseFolder + FNSEPS + svc_name + "_ice.log";
			mIceFileLogger.open( strNSSLog.c_str() ,
				iLogLevel,
				iLogMaxCount,
				iLogSize,
				iLogBufferSize,
				iLogFlushTimeout);
		}
	}
	catch( const ZQ::common::FileLogException& ex)
	{
		glog(ZQ::common::Log::L_ERROR,
			CLOGFMT(C2SSService,"failed to open log file[%s] because [%s]"),
			strNSSLog.c_str(),
			ex.what() );
		return false;
	}
	return true;
}

void C2SSService::uninitializeLogger( )
{

}

bool C2SSService::InitializeIceRunTime( )
{
	try
	{
		Ice::InitializationData		iceInitData;
		int i = 0;
		iceInitData.properties =Ice::createProperties( i , NULL );

		std::stringstream ss;
		ss << pC2SSBaseConfig->_bindDispatchSize;
		iceInitData.properties->setProperty( ADAPTER_NAME_NSS".ThreadPool.Size", ss.str());
		ss << pC2SSBaseConfig->_bindDispatchMax;
		iceInitData.properties->setProperty( ADAPTER_NAME_NSS".ThreadPool.SizeMax",ss.str());
		glog(ZQ::common::Log::L_INFO,CLOGFMT(C2SSService,"set dispatchSize/dispatchSizeMax to [%d/%d]"),
			pC2SSBaseConfig->_bindDispatchSize,
			pC2SSBaseConfig->_bindDispatchMax);
		//StreamService

		//set ice properties

		std::map<std::string, std::string>::const_iterator it = pC2SSBaseConfig->_icePropertiesMap.begin();
		for(; it != pC2SSBaseConfig->_icePropertiesMap.end(); it++)
		{
			iceInitData.properties->setProperty(it->first, it->second);
			glog(ZQ::common::Log::L_INFO,CLOGFMT(C2SSService,"Set ice property [%s] \t\t\t= [%s]"),
				it->first.c_str() , it->second.c_str() );

		}
		if( gC2SSConfig._iceTraceLogEnabled >= 1 )
		{
			iceInitData.logger = new TianShanIce::common::IceLogI( &mIceFileLogger );
			assert( iceInitData.logger );
		}

		mIc	=	Ice::initialize( i , NULL , iceInitData );
		return ( mIc );
	}
	catch( const Ice::Exception& ex )
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(C2SSService, "[%s] caught when InitializeIceRunTime"), ex.ice_name().c_str());
		return false;
	}
	catch(...)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(C2SSService, "unexpected exception caught when InitializeIceRunTime"));
		return false;
	}
}

void C2SSService::UninitializeIceRunTime()
{
	try
	{
		mpC2SSEnv->iceCommunicator = NULL;
		if(mIc)
		{
			mIc->destroy();
			mIc = NULL;
		}
	}
	catch(...)	{}
}

bool C2SSService::initializeServiceParameter( )
{

	//
	// init adapter
	//	
	try
	{
		mainAdapter = ZQADAPTER_CREATE( mIc,"C2SS" ,pC2SSBaseConfig->_bindEndpoint.c_str(), *m_pReporter);
		assert(mainAdapter);
	}
	catch(Ice::Exception& ex)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(C2SSService, "Create adapter failed with endpoint=%s and exception is %s"),
			pC2SSBaseConfig->_bindEndpoint.c_str(), ex.ice_name().c_str());
		return false;
	}

#ifdef ZQ_OS_MSWIN
	mpC2SSEnv->setServiceName(servname);
#else
	mpC2SSEnv->setServiceName(getServiceName().c_str());
#endif
	//
	// init streamservice
	//
	mpC2SSEnv->mainAdapter		= mainAdapter;
	mpC2SSEnv->iceCommunicator	= mIc;

	try
	{
		mpC2SSEnv->init();
	}
	catch( const Ice::Exception& ex )
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(C2SSService, "failed to initialize StreamService, caught[%s]"), ex.ice_name().c_str());
		return false;
	}
	catch(...)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(C2SSService, "failed to initialize StreamService, caught runtime exception"));
		return false;
	}

	glog(ZQ::common::Log::L_INFO, CLOGFMT(C2SSService, "StreamService module initialized successfully"));

	return true;
}

HRESULT C2SSService::OnInit( )
{
	//get regedit argument
	std::string strNetId = "";
	if(m_argc == 1 && strlen(m_argv[0]))
	{
		strNetId = m_argv[0];
	}	


	glog(ZQ::common::Log::L_INFO, CLOGFMT(C2SSService, "netID='%s'"), strNetId.c_str());

	for (::ZQTianShan::C2SS::C2SSCfg::C2SSList::iterator iter = gC2SSConfig._c2ssList.begin(); iter != gC2SSConfig._c2ssList.end(); iter++)
	{
		if (strNetId == iter->_netId)
		{
			glog(ZQ::common::Log::L_INFO, CLOGFMT(C2SSService, "find NetID=%s"), strNetId.c_str());
			pC2SSBaseConfig = &(*iter);
		}
	}

	if (NULL == pC2SSBaseConfig)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(C2SSService, "couldn't find specified netID[%s]"), strNetId.c_str());
		return S_FALSE;
	}

	mStreamServiceThreadpool.resize( pC2SSBaseConfig->_bindThreadPoolSize );

	_thrdPoolRTSP.resize(max(pC2SSBaseConfig->_bindThreadPoolSize/4, 20));

	if ( !initializeCrashDumpLocation() )
	{
#ifdef ZQ_OS_MSWIN
		logEvent(ZQ::common::Log::L_ERROR,_T("failed to initialize crash dump location"));
#endif
		return S_FALSE;
	}

	if (!initializeLogger())
	{
#ifdef ZQ_OS_MSWIN
		logEvent( ZQ::common::Log::L_ERROR,_T("failed to open log"));
#endif
		return S_FALSE;
	}

	mpC2SSEnv = new ZQTianShan::C2SS::C2SSEnv(*m_pReporter, mSessionLogger, _thrdPoolRTSP, mStreamServiceThreadpool);
	mpC2SSEnv->setNetId(strNetId);

	// initialize ice run time
	if( !InitializeIceRunTime() )
	{
#ifdef ZQ_OS_MSWIN
		logEvent(ZQ::common::Log::L_ERROR, _T("failed to initialize ice run time"));
#endif
		return S_FALSE;
	}	

	if (!initializeServiceParameter())
	{
#ifdef ZQ_OS_MSWIN
		logEvent(ZQ::common::Log::L_ERROR, _T("failed to initialize service parameter"));
#endif
		return S_FALSE;
	}	

	ZQ::StreamService::gSsEnvironment	=	mpC2SSEnv;
	mpServiceImpl = new ZQ::StreamService::SsServiceImpl( mpC2SSEnv , "C2SS" );	
	ZQ::StreamService::pServiceInstance	= mpServiceImpl;
	mpServiceImpl->strCheckpointPeriod	=	pC2SSBaseConfig->_videoServerHolder.streamDbPerfTune.databaseStreamCheckpointPeriod;
	mpServiceImpl->strSavePeriod		=	pC2SSBaseConfig->_videoServerHolder.streamDbPerfTune.databaseStreamSavePeriod;
	mpServiceImpl->strSaveSizeTrigger	=	pC2SSBaseConfig->_videoServerHolder.streamDbPerfTune.databaseStreamSaveSizeTrigger;

	//if(NULL == nssSnmpAgent)
	//	nssSnmpAgent = new ZQ::Snmp::Subagent(1400, 5, Application->getInstanceId());

	//using namespace ZQ::Snmp;
	//nssSnmpAgent->addObject(Oid("1.2"), ManagedPtr(new SimpleObject(VariablePtr(new nssSessionCount(mpServiceImpl)), AsnType_Integer, aReadWrite)));
	
	//init FixedSpeedSet
	std::vector<std::string> temps;
	std::vector<float>& inputFFs = pC2SSBaseConfig->_videoServerHolder.sessionInterfaceHolder.FixedSpeedSetForwardSet;
	std::string strInputFF = pC2SSBaseConfig->_videoServerHolder.sessionInterfaceHolder.FixedSpeedSetForward;
	temps.clear();
	ZQ::common::stringHelper::SplitString(strInputFF,temps," "," ","","");
	for( std::vector<std::string>::const_iterator it = temps.begin() ; it != temps.end() ; it ++ )
		inputFFs.push_back((float)atof(it->c_str()));

	std::vector<float>& inputREWs = pC2SSBaseConfig->_videoServerHolder.sessionInterfaceHolder.FixedSpeedSetBackwardSet;
	std::string strInputREW = pC2SSBaseConfig->_videoServerHolder.sessionInterfaceHolder.FixedSpeedSetBackward;
	temps.clear();
	ZQ::common::stringHelper::SplitString(strInputREW,temps," "," ","","");
	for( std::vector<std::string>::const_iterator it = temps.begin() ; it != temps.end() ; it ++ )
		inputREWs.push_back((float)atof(it->c_str()));

	return ZQ::common::BaseZQServiceApplication::OnInit();
}

HRESULT C2SSService::OnStart()
{
	if ( strlen( gC2SSConfig._szIceRuntimeDbFolder ) <= 0)
		strcpy(gC2SSConfig._szIceRuntimeDbFolder,gC2SSConfig._szIceDbFolder);
		//gC2SSConfig._szIceRuntimeDbFolder = gC2SSConfig._szIceDbFolder;
	std::string dbPath = ZQTianShan::Util::fsConcatPath( gC2SSConfig._szIceRuntimeDbFolder, mpC2SSEnv->_strServiceName);

	if (!mpServiceImpl->start(dbPath, gC2SSConfig._eventChannelEndpoint, "", mpC2SSEnv->_netId, mpC2SSEnv->_strServiceName))
	{
		glog(ZQ::common::Log::L_EMERG,CLOGFMT(C2SSService,"failed to start service instance"));
		return -1;
	}

	//if (NULL != nssSnmpAgent)
	//{
	//	nssSnmpAgent->setLogger(m_pReporter);
	//	nssSnmpAgent->start();
	//	glog(ZQ::common::Log::L_INFO, CLOGFMT(C2SSService, "start(), snmp agent succeed"));
	//}
	//else
	//	glog(ZQ::common::Log::L_INFO, CLOGFMT(C2SSService, "start(), snmp agent failed"));

	//ZQ::common::RTSPSession::startDefaultSessionManager();

#ifndef _INDEPENDENT_ADAPTER
	{ // publish logs
		for(size_t i = 0; i < pC2SSBaseConfig->_publishedLog._logDatas.size(); ++i) {
			const ZQTianShan::C2SS::PublishLog& pl= pC2SSBaseConfig->_publishedLog._logDatas[i];
			if(pl._path.empty() || pl._syntax.empty() || pl._key.empty()) {
				glog(ZQ::common::Log::L_WARNING, CLOGFMT(C2SSService,"publishLogger() Bad logger configuration. name[%s] synax[%s] key[%s] type[%s]"), pl._path.c_str(), pl._syntax.c_str(), pl._key.c_str(), pl._type.c_str());
				continue;
			}
			if(mainAdapter->publishLogger(pl._path.c_str(), pl._syntax.c_str(), pl._key.c_str(), pl._type.c_str())) {
				glog(ZQ::common::Log::L_INFO, CLOGFMT(C2SSService,"publishLogger() Successfully publish logger. name[%s] synax[%s] key[%s] type[%s]"), pl._path.c_str(), pl._syntax.c_str(), pl._key.c_str(), pl._type.c_str());
			} else {
				glog(ZQ::common::Log::L_ERROR, CLOGFMT(C2SSService,"publishLogger() Failed to publish logger. name[%s] synax[%s] key[%s] type[%s]"), pl._path.c_str(), pl._syntax.c_str(), pl._key.c_str(), pl._type.c_str());
			}
		}
	}
#endif

	mpC2SSEnv->start();
	mpC2SSEnv->getMainAdapter()->activate();
	ZQ::common::SystemInfo sysInfo(ZQ::common::getGlogger());
	int cpuNum = sysInfo._cpu.size();
	if (cpuNum <= 0)
			cpuNum = 2;
	std::vector<int> cores;
	for(int i = 0; i < cpuNum; i++)
		cores.push_back(i);
	LibAsync::HttpClient::setup(*(ZQ::common::getGlogger()), cores);
	return ZQ::common::BaseZQServiceApplication::OnStart();
}

HRESULT C2SSService::OnStop()
{
	//if (NULL != nssSnmpAgent)
	//{
	//	nssSnmpAgent->stop();
	//	glog(ZQ::common::Log::L_INFO, CLOGFMT(C2SSService, "OnStop() snmp agent stopped"));
	//}

#ifndef _INDEPENDENT_ADAPTER
	{ // unpublish logs
		for(size_t i = 0; i < pC2SSBaseConfig->_publishedLog._logDatas.size(); ++i) {
			const ZQTianShan::C2SS::PublishLog& pl= pC2SSBaseConfig->_publishedLog._logDatas[i];
			if(pl._path.empty() || pl._syntax.empty() || pl._key.empty()) {
				glog(ZQ::common::Log::L_WARNING, CLOGFMT(C2SSService,"OnStop() Bad logger configuration. name[%s] synax[%s] key[%s] type[%s]"), pl._path.c_str(), pl._syntax.c_str(), pl._key.c_str(), pl._type.c_str());
				continue;
			}
			if(mainAdapter->unpublishLogger(pl._path.c_str())) {
				glog(ZQ::common::Log::L_INFO, CLOGFMT(C2SSService,"unpublishLogger() Successfully unpublish logger. name[%s] synax[%s] key[%s] type[%s]"), pl._path.c_str(), pl._syntax.c_str(), pl._key.c_str(), pl._type.c_str());
			} else {
				glog(ZQ::common::Log::L_ERROR, CLOGFMT(C2SSService,"unpublishLogger() Failed to unpublish logger. name[%s] synax[%s] key[%s] type[%s]"), pl._path.c_str(), pl._syntax.c_str(), pl._key.c_str(), pl._type.c_str());
			}
		}
	}
#endif

	if (mpServiceImpl )
	{
		mpServiceImpl->stop();
		//should I delete service instance or just leave it to Ice runtime ?
		glog(ZQ::common::Log::L_INFO, CLOGFMT(C2SSService,"C2SSService stopped"));
	}

	//mNGODCSEnv.uninitEnv();
	//glog(ZQ::common::Log::L_INFO, CLOGFMT(C2SSService,"ContentStore uninitialized"));

	//ZQ::common::RTSPSession::stopDefaultSessionManager();

	if( mpC2SSEnv->mainAdapter )
		mpC2SSEnv->mainAdapter->deactivate();	
	glog(ZQ::common::Log::L_INFO, CLOGFMT(C2SSService,"Adapter deactivated"));

	mStreamServiceThreadpool.stop();
//	mContentStorethreadpool.stop();
	glog(ZQ::common::Log::L_INFO, CLOGFMT(C2SSService,"ThreadPool stopped"));
	LibAsync::HttpClient::teardown();
	return ZQ::common::BaseZQServiceApplication::OnStop();
}

HRESULT C2SSService::OnUnInit()
{
	UninitializeIceRunTime();
	glog(ZQ::common::Log::L_INFO, CLOGFMT(C2SSService, "Ice runtime destroyed"));

	mpC2SSEnv->uninit();
	glog(ZQ::common::Log::L_INFO, CLOGFMT(C2SSService, "C2SS enviroment uninitialized"));
	if(mpC2SSEnv)
	{		
		delete mpC2SSEnv;
		mpC2SSEnv = NULL;
	}

	//if (NULL != nssSnmpAgent)
	//{
	//	ZQ::Snmp::Subagent*  tmpAgent = nssSnmpAgent;
	//	nssSnmpAgent = NULL;
	//	delete tmpAgent;
	//	glog(ZQ::common::Log::L_INFO, CLOGFMT(C2SSService, "snmp agent uninitialized"));
	//}

	if (mpServiceImpl )
	{
		delete mpServiceImpl;
		mpServiceImpl = NULL;
		glog(ZQ::common::Log::L_INFO, CLOGFMT(C2SSService,"C2SSService uninitialized"));
	}

	uninitializeLogger();
	return ZQ::common::BaseZQServiceApplication::OnUnInit();
}
	} //namespace ZQTianShan
} //namespace NssStrem
